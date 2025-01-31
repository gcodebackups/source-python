//  (C) Copyright 2008-10 Anthony Williams
//  (C) Copyright 2011-2012 Vicente J. Botet Escriba
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_THREAD_FUTURE_HPP
#define BOOST_THREAD_FUTURE_HPP

#include <boost/thread/detail/config.hpp>

// boost::thread::future requires exception handling
// due to boost::exception::exception_ptr dependency

#ifndef BOOST_NO_EXCEPTIONS

#include <boost/detail/scoped_enum_emulation.hpp>
#include <stdexcept>
#include <boost/thread/detail/move.hpp>
#include <boost/thread/detail/async_func.hpp>
#include <boost/thread/thread_time.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/lock_algorithms.hpp>
#include <boost/thread/lock_types.hpp>
#include <boost/exception_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/type_traits/is_fundamental.hpp>
#include <boost/type_traits/is_convertible.hpp>
#include <boost/type_traits/remove_reference.hpp>
#include <boost/type_traits/remove_cv.hpp>
#include <boost/mpl/if.hpp>
#include <boost/config.hpp>
#include <boost/throw_exception.hpp>
#include <algorithm>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/ref.hpp>
#include <boost/scoped_array.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/utility/enable_if.hpp>
#include <list>
#include <boost/next_prior.hpp>
#include <vector>

#include <boost/system/error_code.hpp>
#ifdef BOOST_THREAD_USES_CHRONO
#include <boost/chrono/system_clocks.hpp>
#endif

#if defined BOOST_THREAD_PROVIDES_FUTURE_CTOR_ALLOCATORS
#include <boost/thread/detail/memory.hpp>
#endif

#include <boost/utility/result_of.hpp>
#include <boost/thread/thread.hpp>

#if defined BOOST_THREAD_PROVIDES_FUTURE
#define BOOST_THREAD_FUTURE future
#else
#define BOOST_THREAD_FUTURE unique_future
#endif

namespace boost
{

  //enum class future_errc
  BOOST_SCOPED_ENUM_DECLARE_BEGIN(future_errc)
  {
      broken_promise = 1,
      future_already_retrieved,
      promise_already_satisfied,
      no_state
  }
  BOOST_SCOPED_ENUM_DECLARE_END(future_errc)

  namespace system
  {
    template <>
    struct BOOST_SYMBOL_VISIBLE is_error_code_enum<future_errc> : public true_type {};

    #ifdef BOOST_NO_CXX11_SCOPED_ENUMS
    template <>
    struct BOOST_SYMBOL_VISIBLE is_error_code_enum<future_errc::enum_type> : public true_type { };
    #endif
  }

  //enum class launch
  BOOST_SCOPED_ENUM_DECLARE_BEGIN(launch)
  {
      async = 1,
      deferred = 2,
      any = async | deferred
  }
  BOOST_SCOPED_ENUM_DECLARE_END(launch)

  //enum class future_status
  BOOST_SCOPED_ENUM_DECLARE_BEGIN(future_status)
  {
      ready,
      timeout,
      deferred
  }
  BOOST_SCOPED_ENUM_DECLARE_END(future_status)

  BOOST_THREAD_DECL
  const system::error_category& future_category() BOOST_NOEXCEPT;

  namespace system
  {
    inline
    error_code
    make_error_code(future_errc e) //BOOST_NOEXCEPT
    {
        return error_code(underlying_cast<int>(e), boost::future_category());
    }

    inline
    error_condition
    make_error_condition(future_errc e) //BOOST_NOEXCEPT
    {
        return error_condition(underlying_cast<int>(e), future_category());
    }
  }

  class BOOST_SYMBOL_VISIBLE future_error
      : public std::logic_error
  {
      system::error_code ec_;
  public:
      future_error(system::error_code ec)
      : logic_error(ec.message()),
        ec_(ec)
      {
      }

      const system::error_code& code() const BOOST_NOEXCEPT
      {
        return ec_;
      }
      const char* what() const BOOST_THREAD_NOEXCEPT_OR_THROW
      {
        return code().message().c_str();
      }

  };

    class BOOST_SYMBOL_VISIBLE future_uninitialized:
        public future_error
    {
    public:
        future_uninitialized() :
          future_error(system::make_error_code(future_errc::no_state))
        {}
    };
    class BOOST_SYMBOL_VISIBLE broken_promise:
        public future_error
    {
    public:
        broken_promise():
          future_error(system::make_error_code(future_errc::broken_promise))
        {}
    };
    class BOOST_SYMBOL_VISIBLE future_already_retrieved:
        public future_error
    {
    public:
        future_already_retrieved():
          future_error(system::make_error_code(future_errc::future_already_retrieved))
        {}
    };
    class BOOST_SYMBOL_VISIBLE promise_already_satisfied:
        public future_error
    {
    public:
        promise_already_satisfied():
          future_error(system::make_error_code(future_errc::promise_already_satisfied))
        {}
    };

    class BOOST_SYMBOL_VISIBLE task_already_started:
        public future_error
    {
    public:
        task_already_started():
        future_error(system::make_error_code(future_errc::promise_already_satisfied))
        {}
    };

        class BOOST_SYMBOL_VISIBLE task_moved:
            public future_error
        {
        public:
            task_moved():
              future_error(system::make_error_code(future_errc::no_state))
            {}
        };

            class promise_moved:
                public future_error
            {
            public:
                  promise_moved():
                  future_error(system::make_error_code(future_errc::no_state))
                {}
            };

    namespace future_state
    {
        enum state { uninitialized, waiting, ready, moved, deferred };
    }

    namespace detail
    {
#if defined BOOST_THREAD_PROVIDES_FUTURE_CONTINUATION
        struct future_continuation_base
        {
          future_continuation_base() {}
          virtual ~future_continuation_base() {}

          virtual void do_continuation(boost::unique_lock<boost::mutex>& ) {};
        private:
          future_continuation_base(future_continuation_base const&);
          future_continuation_base& operator=(future_continuation_base const&);
        };

        template <typename F, typename R, typename C>
        struct future_continuation;

#endif

        struct relocker
        {
            boost::unique_lock<boost::mutex>& lock_;
            bool  unlocked_;

            relocker(boost::unique_lock<boost::mutex>& lk):
                lock_(lk)
            {
                lock_.unlock();
                unlocked_=true;
            }
            ~relocker()
            {
              if (unlocked_) {
                lock_.lock();
              }
            }
            void lock() {
              if (unlocked_) {
                lock_.lock();
                unlocked_=false;
              }
            }
        private:
            relocker& operator=(relocker const&);
        };

        struct future_object_base : enable_shared_from_this<future_object_base>
        {

            boost::exception_ptr exception;
            bool done;
            bool is_deferred_;
            launch policy_;
            bool is_constructed;
#if defined BOOST_THREAD_PROVIDES_INTERRUPTIONS
            bool thread_was_interrupted;
#endif
            boost::mutex mutex;
            boost::condition_variable waiters;
            typedef std::list<boost::condition_variable_any*> waiter_list;
            waiter_list external_waiters;
            boost::function<void()> callback;
#if defined BOOST_THREAD_PROVIDES_FUTURE_CONTINUATION
            shared_ptr<future_continuation_base> continuation_ptr;
#endif
            future_object_base():
                done(false),
                is_deferred_(false),
                policy_(launch::any),
                is_constructed(false)
#if defined BOOST_THREAD_PROVIDES_INTERRUPTIONS
               , thread_was_interrupted(false)
#endif
#if defined BOOST_THREAD_PROVIDES_FUTURE_CONTINUATION
               , continuation_ptr()
#endif
            {}
            virtual ~future_object_base()
            {}

            void set_deferred() {is_deferred_ = true;}
            void set_launch_policy(launch policy) {policy_ = policy;}

            waiter_list::iterator register_external_waiter(boost::condition_variable_any& cv)
            {
                boost::unique_lock<boost::mutex> lock(mutex);
                do_callback(lock);
                return external_waiters.insert(external_waiters.end(),&cv);
            }

            void remove_external_waiter(waiter_list::iterator it)
            {
                boost::lock_guard<boost::mutex> lock(mutex);
                external_waiters.erase(it);
            }

#if defined BOOST_THREAD_PROVIDES_FUTURE_CONTINUATION
            void do_continuation(boost::unique_lock<boost::mutex>& lock)
            {
                if (continuation_ptr) {
                  continuation_ptr->do_continuation(lock);
                }
            }
#else
            void do_continuation(boost::unique_lock<boost::mutex>&)
            {
            }
#endif
#if defined BOOST_THREAD_PROVIDES_FUTURE_CONTINUATION
            void set_continuation_ptr(future_continuation_base* continuation, boost::unique_lock<boost::mutex>& lock)
            {
              continuation_ptr.reset(continuation);
              if (done) {
                do_continuation(lock);
              }
            }
#endif
            void mark_finished_internal(boost::unique_lock<boost::mutex>& lock)
            {
                done=true;
                waiters.notify_all();
                for(waiter_list::const_iterator it=external_waiters.begin(),
                        end=external_waiters.end();it!=end;++it)
                {
                    (*it)->notify_all();
                }
                do_continuation(lock);
            }
            void make_ready()
            {
              boost::unique_lock<boost::mutex> lock(mutex);
              mark_finished_internal(lock);
            }

            void do_callback(boost::unique_lock<boost::mutex>& lock)
            {
                if(callback && !done)
                {
                    boost::function<void()> local_callback=callback;
                    relocker relock(lock);
                    local_callback();
                }
            }


            void wait_internal(boost::unique_lock<boost::mutex> &lock, bool rethrow=true)
            {
              do_callback(lock);
              //if (!done) // fixme why this doesn't works?
              {
                if (is_deferred_)
                {
                  is_deferred_=false;
                  execute(lock);
                  //lock.unlock();
                }
                else
                {
                  while(!done)
                  {
                      waiters.wait(lock);
                  }
#if defined BOOST_THREAD_PROVIDES_INTERRUPTIONS
                  if(rethrow && thread_was_interrupted)
                  {
                      throw boost::thread_interrupted();
                  }
#endif
                  if(rethrow && exception)
                  {
                      boost::rethrow_exception(exception);
                  }
                }
              }
            }
            void wait(bool rethrow=true)
            {
                boost::unique_lock<boost::mutex> lock(mutex);
                wait_internal(lock, rethrow);
            }

#if defined BOOST_THREAD_USES_DATETIME
            bool timed_wait_until(boost::system_time const& target_time)
            {
                boost::unique_lock<boost::mutex> lock(mutex);
                if (is_deferred_)
                    return false;

                do_callback(lock);
                while(!done)
                {
                    bool const success=waiters.timed_wait(lock,target_time);
                    if(!success && !done)
                    {
                        return false;
                    }
                }
                return true;
            }
#endif
#ifdef BOOST_THREAD_USES_CHRONO

            template <class Clock, class Duration>
            future_status
            wait_until(const chrono::time_point<Clock, Duration>& abs_time)
            {
              boost::unique_lock<boost::mutex> lock(mutex);
              if (is_deferred_)
                  return future_status::deferred;
              do_callback(lock);
              while(!done)
              {
                  cv_status const st=waiters.wait_until(lock,abs_time);
                  if(st==cv_status::timeout && !done)
                  {
                    return future_status::timeout;
                  }
              }
              return future_status::ready;
            }
#endif
            void mark_exceptional_finish_internal(boost::exception_ptr const& e, boost::unique_lock<boost::mutex>& lock)
            {
                exception=e;
                mark_finished_internal(lock);
            }
            void mark_exceptional_finish()
            {
                boost::unique_lock<boost::mutex> lock(mutex);
                mark_exceptional_finish_internal(boost::current_exception(), lock);
            }
#if defined BOOST_THREAD_PROVIDES_INTERRUPTIONS
            void mark_interrupted_finish()
            {
                boost::unique_lock<boost::mutex> lock(mutex);
                thread_was_interrupted=true;
                mark_finished_internal(lock);
            }
            void set_interrupted_at_thread_exit()
            {
              unique_lock<boost::mutex> lk(mutex);
              thread_was_interrupted=true;
              if (has_value(lk))
              {
                  throw_exception(promise_already_satisfied());
              }
              get_current_thread_data()->make_ready_at_thread_exit(shared_from_this());
            }
#endif
            void set_exception_at_thread_exit(exception_ptr e)
            {
              unique_lock<boost::mutex> lk(mutex);
              if (has_value(lk))
              {
                  throw_exception(promise_already_satisfied());
              }
              exception=e;
              get_current_thread_data()->make_ready_at_thread_exit(shared_from_this());
            }
            bool has_value()
            {
                boost::lock_guard<boost::mutex> lock(mutex);
                return done && !(exception
#if defined BOOST_THREAD_PROVIDES_INTERRUPTIONS
                    || thread_was_interrupted
#endif
                );
            }
            bool has_value(unique_lock<boost::mutex>& )
            {
                return done && !(exception
#if defined BOOST_THREAD_PROVIDES_INTERRUPTIONS
                    || thread_was_interrupted
#endif
                );
            }
            bool has_exception()
            {
                boost::lock_guard<boost::mutex> lock(mutex);
                return done && (exception
#if defined BOOST_THREAD_PROVIDES_INTERRUPTIONS
                    || thread_was_interrupted
#endif
                    );
            }
            bool has_exception(unique_lock<boost::mutex>&)
            {
                return done && (exception
#if defined BOOST_THREAD_PROVIDES_INTERRUPTIONS
                    || thread_was_interrupted
#endif
                    );
            }
            void is_deferred() {is_deferred_ = true;}

            launch launch_policy() const BOOST_NOEXCEPT
            {
                return policy_;
            }

            template<typename F,typename U>
            void set_wait_callback(F f,U* u)
            {
                callback=boost::bind(f,boost::ref(*u));
            }
            virtual void execute(boost::unique_lock<boost::mutex>&) {}

        private:
            future_object_base(future_object_base const&);
            future_object_base& operator=(future_object_base const&);
        };

        template<typename T>
        struct future_traits
        {
            typedef boost::scoped_ptr<T> storage_type;
#ifndef BOOST_NO_CXX11_RVALUE_REFERENCES
            typedef T const& source_reference_type;
            struct dummy;
            typedef typename boost::mpl::if_<boost::is_fundamental<T>,dummy&,BOOST_THREAD_RV_REF(T)>::type rvalue_source_type;
            typedef typename boost::mpl::if_<boost::is_fundamental<T>,T,BOOST_THREAD_RV_REF(T)>::type move_dest_type;
#elif defined BOOST_THREAD_USES_MOVE
            typedef T& source_reference_type;
            typedef typename boost::mpl::if_<boost::has_move_emulation_enabled<T>,BOOST_THREAD_RV_REF(T),T const&>::type rvalue_source_type;
            typedef typename boost::mpl::if_<boost::has_move_emulation_enabled<T>,BOOST_THREAD_RV_REF(T),T>::type move_dest_type;
#else
            typedef T& source_reference_type;
            typedef typename boost::mpl::if_<boost::is_convertible<T&,BOOST_THREAD_RV_REF(T) >,BOOST_THREAD_RV_REF(T),T const&>::type rvalue_source_type;
            typedef typename boost::mpl::if_<boost::is_convertible<T&,BOOST_THREAD_RV_REF(T) >,BOOST_THREAD_RV_REF(T),T>::type move_dest_type;
#endif

            typedef const T& shared_future_get_result_type;

            static void init(storage_type& storage,source_reference_type t)
            {
                storage.reset(new T(t));
            }

            static void init(storage_type& storage,rvalue_source_type t)
            {
              storage.reset(new T(static_cast<rvalue_source_type>(t)));
            }

            static void cleanup(storage_type& storage)
            {
                storage.reset();
            }
        };

        template<typename T>
        struct future_traits<T&>
        {
            typedef T* storage_type;
            typedef T& source_reference_type;
            struct rvalue_source_type
            {};
            typedef T& move_dest_type;
            typedef T& shared_future_get_result_type;

            static void init(storage_type& storage,T& t)
            {
                storage=&t;
            }

            static void cleanup(storage_type& storage)
            {
                storage=0;
            }
        };

        template<>
        struct future_traits<void>
        {
            typedef bool storage_type;
            typedef void move_dest_type;
            typedef void shared_future_get_result_type;

            static void init(storage_type& storage)
            {
                storage=true;
            }

            static void cleanup(storage_type& storage)
            {
                storage=false;
            }

        };

        // Used to create stand-alone futures
        template<typename T>
        struct future_object:
            detail::future_object_base
        {
            typedef typename future_traits<T>::storage_type storage_type;
            typedef typename future_traits<T>::source_reference_type source_reference_type;
            typedef typename future_traits<T>::rvalue_source_type rvalue_source_type;
            typedef typename future_traits<T>::move_dest_type move_dest_type;
            typedef typename future_traits<T>::shared_future_get_result_type shared_future_get_result_type;

            storage_type result;

            future_object():
                result(0)
            {}

            ~future_object()
            {
            }

            void mark_finished_with_result_internal(source_reference_type result_, boost::unique_lock<boost::mutex>& lock)
            {
                future_traits<T>::init(result,result_);
                mark_finished_internal(lock);
            }

            void mark_finished_with_result_internal(rvalue_source_type result_, boost::unique_lock<boost::mutex>& lock)
            {
                future_traits<T>::init(result,static_cast<rvalue_source_type>(result_));
                mark_finished_internal(lock);
            }

            void mark_finished_with_result(source_reference_type result_)
            {
                boost::unique_lock<boost::mutex> lock(mutex);
                mark_finished_with_result_internal(result_, lock);
            }

            void mark_finished_with_result(rvalue_source_type result_)
            {
                boost::unique_lock<boost::mutex> lock(mutex);
                mark_finished_with_result_internal(static_cast<rvalue_source_type>(result_), lock);
            }


            move_dest_type get()
            {
                wait();
                return static_cast<move_dest_type>(*result);
            }

            shared_future_get_result_type get_sh()
            {
                wait();
                return static_cast<shared_future_get_result_type>(*result);
            }

            // todo move this to detail::future_object_base
            future_state::state get_state()
            {
                boost::lock_guard<boost::mutex> guard(mutex);
                if(!done)
                {
                    return future_state::waiting;
                }
                else
                {
                    return future_state::ready;
                }
            }


            void set_value_at_thread_exit(const T & result_)
            {
              unique_lock<boost::mutex> lk(this->mutex);
              if (this->has_value(lk))
              {
                  throw_exception(promise_already_satisfied());
              }
              future_traits<T>::init(result,result_);
              this->is_constructed = true;
              get_current_thread_data()->make_ready_at_thread_exit(shared_from_this());
            }
            void set_value_at_thread_exit(BOOST_THREAD_RV_REF(T) result_)
            {
              unique_lock<boost::mutex> lk(this->mutex);
              if (this->has_value(lk))
                  throw_exception(promise_already_satisfied());
              result.reset(new T(boost::move(result_)));
              //future_traits<T>::init(result,static_cast<rvalue_source_type>(result_));
              this->is_constructed = true;
              get_current_thread_data()->make_ready_at_thread_exit(shared_from_this());
            }


        private:
            future_object(future_object const&);
            future_object& operator=(future_object const&);
        };

        template<typename T>
        struct future_object<T&>:
            detail::future_object_base
        {
            typedef typename future_traits<T>::storage_type storage_type;
            typedef typename future_traits<T>::source_reference_type source_reference_type;
            typedef typename future_traits<T>::rvalue_source_type rvalue_source_type;
            typedef typename future_traits<T>::move_dest_type move_dest_type;
            typedef typename future_traits<T>::shared_future_get_result_type shared_future_get_result_type;

            T* result;

            future_object():
                result(0)
            {}

            ~future_object()
            {
            }

            void mark_finished_with_result_internal(T& result_, boost::unique_lock<boost::mutex>& lock)
            {
                //future_traits<T>::init(result,result_);
                result= &result_;
                mark_finished_internal(lock);
            }

//            void mark_finished_with_result_internal(rvalue_source_type result_, boost::unique_lock<boost::mutex>& lock)
//            {
//                future_traits<T>::init(result,static_cast<rvalue_source_type>(result_));
//                mark_finished_internal(lock);
//            }

            void mark_finished_with_result(T& result_)
            {
                boost::unique_lock<boost::mutex> lock(mutex);
                mark_finished_with_result_internal(result_, lock);
            }

//            void mark_finished_with_result(rvalue_source_type result_)
//            {
//                boost::unique_lock<boost::mutex> lock(mutex);
//                mark_finished_with_result_internal(static_cast<rvalue_source_type>(result_), lock);
//            }


            T& get()
            {
                wait();
                //return static_cast<T&>(*result);
                return *result;
            }

            T& get_sh()
            {
                wait();
                //return static_cast<shared_future_get_result_type>(*result);
                return *result;
            }

            // todo move this to detail::future_object_base
            future_state::state get_state()
            {
                boost::lock_guard<boost::mutex> guard(mutex);
                if(!done)
                {
                    return future_state::waiting;
                }
                else
                {
                    return future_state::ready;
                }
            }

            void set_value_at_thread_exit(T& result_)
            {
              unique_lock<boost::mutex> lk(this->mutex);
              if (this->has_value(lk))
                  throw_exception(promise_already_satisfied());
              //future_traits<T>::init(result,result_);
              result= &result_;
              this->is_constructed = true;
              get_current_thread_data()->make_ready_at_thread_exit(shared_from_this());
            }
//            void set_value_at_thread_exit(rvalue_source_type result_)
//            {
//              unique_lock<boost::mutex> lk(this->mutex);
//              if (this->has_value())
//                  throw_exception(promise_already_satisfied());
//              future_traits<T>::init(result,static_cast<rvalue_source_type>(result_));
//              this->is_constructed = true;
//              get_current_thread_data()->make_ready_at_thread_exit(shared_from_this());
//            }


        private:
            future_object(future_object const&);
            future_object& operator=(future_object const&);
        };

        template<>
        struct future_object<void>:
            detail::future_object_base
        {
          typedef void shared_future_get_result_type;

            future_object()
            {}

            void mark_finished_with_result_internal(boost::unique_lock<boost::mutex>& lock)
            {
                mark_finished_internal(lock);
            }

            void mark_finished_with_result()
            {
                boost::unique_lock<boost::mutex> lock(mutex);
                mark_finished_with_result_internal(lock);
            }

            void get()
            {
                this->wait();
            }
            void get_sh()
            {
                wait();
            }
            // todo move this to detail::future_object_base
            future_state::state get_state()
            {
                boost::lock_guard<boost::mutex> guard(mutex);
                if(!done)
                {
                    return future_state::waiting;
                }
                else
                {
                    return future_state::ready;
                }
            }
            void set_value_at_thread_exit()
            {
              unique_lock<boost::mutex> lk(this->mutex);
              if (this->has_value(lk))
              {
                  throw_exception(promise_already_satisfied());
              }
             this->is_constructed = true;
              get_current_thread_data()->make_ready_at_thread_exit(shared_from_this());
            }
        private:
            future_object(future_object const&);
            future_object& operator=(future_object const&);
        };

        /// future_async_object
        template<typename Rp, typename Fp>
        struct future_async_object: future_object<Rp>
        {
          typedef future_object<Rp> base_type;
          Fp func_;
          boost::thread thr_;

        public:
        #if ! defined BOOST_NO_CXX11_RVALUE_REFERENCES
          explicit future_async_object(Fp&& f)
          : func_(boost::forward<Fp>(f))
        #else
          explicit future_async_object(Fp f)
          : func_(f)
        #endif
          , thr_(&future_async_object::execute, *this)
          {
          }

          ~future_async_object()
          {
            thr_.join();
          }

          virtual void execute(boost::unique_lock<boost::mutex>& lock) {
            try
            {
              this->mark_finished_with_result_internal(func_(), lock);
            }
            catch (...)
            {
              this->mark_exceptional_finish_internal(current_exception(), lock);
            }

          }

        };

        template<typename Fp>
        struct future_async_object<void, Fp>: public future_object<void>
        {
          typedef future_object<void> base_type;
          Fp func_;
          boost::thread thr_;

        public:
        #if ! defined BOOST_NO_CXX11_RVALUE_REFERENCES
          explicit future_async_object(Fp&& f)
          : func_(boost::forward<Fp>(f))
        #else
          explicit future_async_object(Fp f)
          : func_(f)
        #endif
          , thr_(&future_async_object::execute, *this)
          {
          }

          ~future_async_object()
          {
            thr_.join();
          }

          virtual void execute(boost::unique_lock<boost::mutex>& lock) {
            try
            {
              func_();
              this->mark_finished_with_result_internal(lock);
            }
            catch (...)
            {
              this->mark_exceptional_finish_internal(current_exception(), lock);
            }

          }

        };
        /// future_deferred_object
        template<typename Rp, typename Fp>
        struct future_deferred_object: future_object<Rp>
        {
          typedef future_object<Rp> base_type;
          Fp func_;

        public:
        #if ! defined BOOST_NO_CXX11_RVALUE_REFERENCES
          explicit future_deferred_object(Fp&& f)
          : func_(boost::forward<Fp>(f))
        #else
          explicit future_deferred_object(Fp f)
          : func_(f)
        #endif
          {
            this->set_deferred();
          }

          virtual void execute(boost::unique_lock<boost::mutex>& lck) {
            try
            {
              this->mark_finished_with_result_internal(func_(), lck);
            }
            catch (...)
            {
              this->mark_exceptional_finish_internal(current_exception(), lck);
            }
          }
        };

        template<typename Fp>
        struct future_deferred_object<void,Fp>: future_object<void>
        {
          typedef future_object<void> base_type;
          Fp func_;

        public:
        #if ! defined BOOST_NO_CXX11_RVALUE_REFERENCES
          explicit future_deferred_object(Fp&& f)
          : func_(boost::forward<Fp>(f))
        #else
          explicit future_deferred_object(Fp f)
          : func_(f)
        #endif
          {
            this->set_deferred();
          }

          virtual void execute(boost::unique_lock<boost::mutex>& lck) {
            try
            {
              func_();
              this->mark_finished_with_result_internal(lck);
            }
            catch (...)
            {
              this->mark_exceptional_finish_internal(current_exception(), lck);
            }
          }
        };

//        template<typename T, typename Allocator>
//        struct future_object_alloc: public future_object<T>
//        {
//          typedef future_object<T> base;
//          Allocator alloc_;
//
//        public:
//          explicit future_object_alloc(const Allocator& a)
//              : alloc_(a) {}
//
//        };
        class future_waiter
        {
            struct registered_waiter;
            typedef std::vector<int>::size_type count_type;

            struct registered_waiter
            {
                boost::shared_ptr<detail::future_object_base> future_;
                detail::future_object_base::waiter_list::iterator wait_iterator;
                count_type index;

                registered_waiter(boost::shared_ptr<detail::future_object_base> const& a_future,
                                  detail::future_object_base::waiter_list::iterator wait_iterator_,
                                  count_type index_):
                    future_(a_future),wait_iterator(wait_iterator_),index(index_)
                {}
            };

            struct all_futures_lock
            {
#ifdef _MANAGED
                   typedef std::ptrdiff_t count_type_portable;
#else
                   typedef count_type count_type_portable;
#endif
                   count_type_portable count;
                   boost::scoped_array<boost::unique_lock<boost::mutex> > locks;

                all_futures_lock(std::vector<registered_waiter>& futures):
                    count(futures.size()),locks(new boost::unique_lock<boost::mutex>[count])
                {
                    for(count_type_portable i=0;i<count;++i)
                    {
#if defined __DECCXX || defined __SUNPRO_CC || defined __hpux
                        locks[i]=boost::unique_lock<boost::mutex>(futures[i].future_->mutex).move();
#else
                        locks[i]=boost::unique_lock<boost::mutex>(futures[i].future_->mutex); // TODO shouldn't be moved explicitly
#endif
                    }
                }

                void lock()
                {
                    boost::lock(locks.get(),locks.get()+count);
                }

                void unlock()
                {
                    for(count_type_portable i=0;i<count;++i)
                    {
                        locks[i].unlock();
                    }
                }
            };

            boost::condition_variable_any cv;
            std::vector<registered_waiter> futures;
            count_type future_count;

        public:
            future_waiter():
                future_count(0)
            {}

            template<typename F>
            void add(F& f)
            {
                if(f.future_)
                {
                    futures.push_back(registered_waiter(f.future_,f.future_->register_external_waiter(cv),future_count));
                }
                ++future_count;
            }

            count_type wait()
            {
                all_futures_lock lk(futures);
                for(;;)
                {
                    for(count_type i=0;i<futures.size();++i)
                    {
                        if(futures[i].future_->done)
                        {
                            return futures[i].index;
                        }
                    }
                    cv.wait(lk);
                }
            }

            ~future_waiter()
            {
                for(count_type i=0;i<futures.size();++i)
                {
                    futures[i].future_->remove_external_waiter(futures[i].wait_iterator);
                }
            }

        };

    }

    template <typename R>
    class BOOST_THREAD_FUTURE;

    template <typename R>
    class shared_future;

    template<typename T>
    struct is_future_type
    {
        BOOST_STATIC_CONSTANT(bool, value=false);
    };

    template<typename T>
    struct is_future_type<BOOST_THREAD_FUTURE<T> >
    {
        BOOST_STATIC_CONSTANT(bool, value=true);
    };

    template<typename T>
    struct is_future_type<shared_future<T> >
    {
        BOOST_STATIC_CONSTANT(bool, value=true);
    };

    template<typename Iterator>
    typename boost::disable_if<is_future_type<Iterator>,void>::type wait_for_all(Iterator begin,Iterator end)
    {
        for(Iterator current=begin;current!=end;++current)
        {
            current->wait();
        }
    }

    template<typename F1,typename F2>
    typename boost::enable_if<is_future_type<F1>,void>::type wait_for_all(F1& f1,F2& f2)
    {
        f1.wait();
        f2.wait();
    }

    template<typename F1,typename F2,typename F3>
    void wait_for_all(F1& f1,F2& f2,F3& f3)
    {
        f1.wait();
        f2.wait();
        f3.wait();
    }

    template<typename F1,typename F2,typename F3,typename F4>
    void wait_for_all(F1& f1,F2& f2,F3& f3,F4& f4)
    {
        f1.wait();
        f2.wait();
        f3.wait();
        f4.wait();
    }

    template<typename F1,typename F2,typename F3,typename F4,typename F5>
    void wait_for_all(F1& f1,F2& f2,F3& f3,F4& f4,F5& f5)
    {
        f1.wait();
        f2.wait();
        f3.wait();
        f4.wait();
        f5.wait();
    }

    template<typename Iterator>
    typename boost::disable_if<is_future_type<Iterator>,Iterator>::type wait_for_any(Iterator begin,Iterator end)
    {
        if(begin==end)
            return end;

        detail::future_waiter waiter;
        for(Iterator current=begin;current!=end;++current)
        {
            waiter.add(*current);
        }
        return boost::next(begin,waiter.wait());
    }

    template<typename F1,typename F2>
    typename boost::enable_if<is_future_type<F1>,unsigned>::type wait_for_any(F1& f1,F2& f2)
    {
        detail::future_waiter waiter;
        waiter.add(f1);
        waiter.add(f2);
        return waiter.wait();
    }

    template<typename F1,typename F2,typename F3>
    unsigned wait_for_any(F1& f1,F2& f2,F3& f3)
    {
        detail::future_waiter waiter;
        waiter.add(f1);
        waiter.add(f2);
        waiter.add(f3);
        return waiter.wait();
    }

    template<typename F1,typename F2,typename F3,typename F4>
    unsigned wait_for_any(F1& f1,F2& f2,F3& f3,F4& f4)
    {
        detail::future_waiter waiter;
        waiter.add(f1);
        waiter.add(f2);
        waiter.add(f3);
        waiter.add(f4);
        return waiter.wait();
    }

    template<typename F1,typename F2,typename F3,typename F4,typename F5>
    unsigned wait_for_any(F1& f1,F2& f2,F3& f3,F4& f4,F5& f5)
    {
        detail::future_waiter waiter;
        waiter.add(f1);
        waiter.add(f2);
        waiter.add(f3);
        waiter.add(f4);
        waiter.add(f5);
        return waiter.wait();
    }

    template <typename R>
    class promise;

    template <typename R>
    class packaged_task;

    namespace detail
    {
      /// Common implementation for all the futures independently of the return type
      class base_future
      {
        //BOOST_THREAD_MOVABLE(base_future)

      };
      /// Common implementation for future and shared_future.
      template <typename R>
      class basic_future : public base_future
      {
      protected:

        typedef boost::shared_ptr<detail::future_object<R> > future_ptr;

        future_ptr future_;

        basic_future(future_ptr a_future):
          future_(a_future)
        {
        }
        // Copy construction from a shared_future
        explicit basic_future(const shared_future<R>&) BOOST_NOEXCEPT;

      public:
        typedef future_state::state state;

        BOOST_THREAD_MOVABLE(basic_future)
        basic_future(): future_() {}
        ~basic_future() {}

        basic_future(BOOST_THREAD_RV_REF(basic_future) other) BOOST_NOEXCEPT:
        future_(BOOST_THREAD_RV(other).future_)
        {
            BOOST_THREAD_RV(other).future_.reset();
        }
        basic_future& operator=(BOOST_THREAD_RV_REF(basic_future) other) BOOST_NOEXCEPT
        {
            future_=BOOST_THREAD_RV(other).future_;
            BOOST_THREAD_RV(other).future_.reset();
            return *this;
        }
        void swap(basic_future& that) BOOST_NOEXCEPT
        {
          future_.swap(that.future_);
        }
        // functions to check state, and wait for ready
        state get_state() const BOOST_NOEXCEPT
        {
            if(!future_)
            {
                return future_state::uninitialized;
            }
            return future_->get_state();
        }

        bool is_ready() const BOOST_NOEXCEPT
        {
            return get_state()==future_state::ready;
        }

        bool has_exception() const BOOST_NOEXCEPT
        {
            return future_ && future_->has_exception();
        }

        bool has_value() const BOOST_NOEXCEPT
        {
            return future_ && future_->has_value();
        }

        launch launch_policy() const BOOST_NOEXCEPT
        {
            if ( future_ ) return future_->launch_policy();
            else return launch(launch::any);
        }

        bool valid() const BOOST_NOEXCEPT
        {
            return future_ != 0;
        }


        void wait() const
        {
            if(!future_)
            {
                boost::throw_exception(future_uninitialized());
            }
            future_->wait(false);
        }

#if defined BOOST_THREAD_USES_DATETIME
        template<typename Duration>
        bool timed_wait(Duration const& rel_time) const
        {
            return timed_wait_until(boost::get_system_time()+rel_time);
        }

        bool timed_wait_until(boost::system_time const& abs_time) const
        {
            if(!future_)
            {
                boost::throw_exception(future_uninitialized());
            }
            return future_->timed_wait_until(abs_time);
        }
#endif
#ifdef BOOST_THREAD_USES_CHRONO
        template <class Rep, class Period>
        future_status
        wait_for(const chrono::duration<Rep, Period>& rel_time) const
        {
          return wait_until(chrono::steady_clock::now() + rel_time);

        }
        template <class Clock, class Duration>
        future_status
        wait_until(const chrono::time_point<Clock, Duration>& abs_time) const
        {
          if(!future_)
          {
              boost::throw_exception(future_uninitialized());
          }
          return future_->wait_until(abs_time);
        }
#endif

      };

    } // detail
    BOOST_THREAD_DCL_MOVABLE_BEG(R) detail::basic_future<R> BOOST_THREAD_DCL_MOVABLE_END

    namespace detail
    {
        template <class Rp, class Fp>
        BOOST_THREAD_FUTURE<Rp>
        #if ! defined BOOST_NO_CXX11_RVALUE_REFERENCES
        make_future_async_object(Fp&& f);
        #else
        make_future_async_object(Fp f);
        #endif

        template <class Rp, class Fp>
        BOOST_THREAD_FUTURE<Rp>
        #if ! defined BOOST_NO_CXX11_RVALUE_REFERENCES
        make_future_deferred_object(Fp&& f);
        #else
        make_future_deferred_object(Fp f);
        #endif
    }

    template <typename R>
    class BOOST_THREAD_FUTURE : public detail::basic_future<R>
    {
    private:
        typedef detail::basic_future<R> base_type;
        typedef typename base_type::future_ptr future_ptr;

        friend class shared_future<R>;
        friend class promise<R>;
#if defined BOOST_THREAD_PROVIDES_FUTURE_CONTINUATION
        template <typename, typename, typename>
        friend struct detail::future_continuation;
#endif
#if defined BOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK
        template <class> friend class packaged_task; // todo check if this works in windows
#else
        friend class packaged_task<R>;
#endif
        friend class detail::future_waiter;

        template <class Rp, class Fp>
        friend BOOST_THREAD_FUTURE<Rp>
        #if ! defined BOOST_NO_CXX11_RVALUE_REFERENCES
        detail::make_future_async_object(Fp&& f);
        #else
        detail::make_future_async_object(Fp f);
        #endif

        template <class Rp, class Fp>
        friend BOOST_THREAD_FUTURE<Rp>
        #if ! defined BOOST_NO_CXX11_RVALUE_REFERENCES
        detail::make_future_deferred_object(Fp&& f);
        #else
        detail::make_future_deferred_object(Fp f);
        #endif

        typedef typename detail::future_traits<R>::move_dest_type move_dest_type;

        BOOST_THREAD_FUTURE(future_ptr a_future):
          base_type(a_future)
        {
        }

    public:
        BOOST_THREAD_MOVABLE_ONLY(BOOST_THREAD_FUTURE)
        typedef future_state::state state;

        BOOST_THREAD_FUTURE() {}

        ~BOOST_THREAD_FUTURE() {}

        BOOST_THREAD_FUTURE(BOOST_THREAD_RV_REF(BOOST_THREAD_FUTURE) other) BOOST_NOEXCEPT:
        base_type(boost::move(static_cast<base_type&>(BOOST_THREAD_RV(other))))
        {
        }

        BOOST_THREAD_FUTURE& operator=(BOOST_THREAD_RV_REF(BOOST_THREAD_FUTURE) other) BOOST_NOEXCEPT
        {
            base_type::operator=(boost::move(static_cast<base_type&>(BOOST_THREAD_RV(other))));
            return *this;
        }

        shared_future<R> share()
        {
          return shared_future<R>(::boost::move(*this));
        }

        void swap(BOOST_THREAD_FUTURE& other)
        {
            static_cast<base_type*>(this)->swap(other);
        }

        // retrieving the value
        move_dest_type get()
        {
            if(!this->future_)
            {
                boost::throw_exception(future_uninitialized());
            }
#ifdef BOOST_THREAD_PROVIDES_FUTURE_INVALID_AFTER_GET
            future_ptr fut_=this->future_;
            this->future_.reset();
            return fut_->get();
#else
            return this->future_->get();
#endif
        }


#if defined BOOST_THREAD_PROVIDES_FUTURE_CONTINUATION

//        template<typename F>
//        auto then(F&& func) -> BOOST_THREAD_FUTURE<decltype(func(*this))>;

#if defined(BOOST_THREAD_RVALUE_REFERENCES_DONT_MATCH_FUNTION_PTR)
        template<typename RF>
        inline BOOST_THREAD_FUTURE<RF> then(RF(*func)(BOOST_THREAD_FUTURE&));
        template<typename RF>
        inline BOOST_THREAD_FUTURE<RF> then(launch policy, RF(*func)(BOOST_THREAD_FUTURE&));
#endif
        template<typename F>
        inline BOOST_THREAD_FUTURE<typename boost::result_of<F(BOOST_THREAD_FUTURE&)>::type>
        then(BOOST_THREAD_RV_REF(F) func);
        template<typename F>
        inline BOOST_THREAD_FUTURE<typename boost::result_of<F(BOOST_THREAD_FUTURE&)>::type>
        then(launch policy, BOOST_THREAD_RV_REF(F) func);
#endif
    };

    BOOST_THREAD_DCL_MOVABLE_BEG(T) BOOST_THREAD_FUTURE<T> BOOST_THREAD_DCL_MOVABLE_END

    template <typename R>
    class shared_future : public detail::basic_future<R>
    {

        typedef detail::basic_future<R> base_type;
        typedef typename base_type::future_ptr future_ptr;

        friend class detail::future_waiter;
        friend class promise<R>;

#if defined BOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK
        template <class> friend class packaged_task;// todo check if this works in windows
#else
        friend class packaged_task<R>;
#endif
        shared_future(future_ptr a_future):
          base_type(a_future)
        {}

    public:
        BOOST_THREAD_MOVABLE(shared_future)

        shared_future(shared_future const& other):
        base_type(other)
        {}

        typedef future_state::state state;

        shared_future()
        {}

        ~shared_future()
        {}

        shared_future& operator=(shared_future const& other)
        {
            shared_future(other).swap(*this);
            return *this;
        }
        shared_future(BOOST_THREAD_RV_REF(shared_future) other) BOOST_NOEXCEPT :
        base_type(boost::move(static_cast<base_type&>(BOOST_THREAD_RV(other))))
        {
            BOOST_THREAD_RV(other).future_.reset();
        }
        shared_future(BOOST_THREAD_RV_REF( BOOST_THREAD_FUTURE<R> ) other) BOOST_NOEXCEPT :
        base_type(boost::move(static_cast<base_type&>(BOOST_THREAD_RV(other))))
        {
        }

        shared_future& operator=(BOOST_THREAD_RV_REF(shared_future) other) BOOST_NOEXCEPT
        {
            base_type::operator=(boost::move(static_cast<base_type&>(BOOST_THREAD_RV(other))));
            return *this;
        }
        shared_future& operator=(BOOST_THREAD_RV_REF( BOOST_THREAD_FUTURE<R> ) other) BOOST_NOEXCEPT
        {
            base_type::operator=(boost::move(static_cast<base_type&>(BOOST_THREAD_RV(other))));
            //shared_future(boost::move(other)).swap(*this);
            //this->future_.swap(BOOST_THREAD_RV(other).future_);
            //BOOST_THREAD_RV(other).future_.reset();
            return *this;
        }

        void swap(shared_future& other) BOOST_NOEXCEPT
        {
            static_cast<base_type*>(this)->swap(other);
        }

        // retrieving the value
        typename detail::future_object<R>::shared_future_get_result_type get()
        {
            if(!this->future_)
            {
                boost::throw_exception(future_uninitialized());
            }

            return this->future_->get_sh();
        }

    };

    BOOST_THREAD_DCL_MOVABLE_BEG(T) shared_future<T> BOOST_THREAD_DCL_MOVABLE_END

    namespace detail
    {
      /// Copy construction from a shared_future
      template <typename R>
      inline basic_future<R>::basic_future(const shared_future<R>& other) BOOST_NOEXCEPT
      : future_(other.future_)
      {
      }
    }

    template <typename R>
    class promise
    {
        typedef boost::shared_ptr<detail::future_object<R> > future_ptr;

        future_ptr future_;
        bool future_obtained;

        void lazy_init()
        {
#if defined BOOST_THREAD_PROVIDES_PROMISE_LAZY
            if(!atomic_load(&future_))
            {
                future_ptr blank;
                atomic_compare_exchange(&future_,&blank,future_ptr(new detail::future_object<R>));
            }
#endif
        }

    public:
        BOOST_THREAD_MOVABLE_ONLY(promise)
#if defined BOOST_THREAD_PROVIDES_FUTURE_CTOR_ALLOCATORS
        template <class Allocator>
        promise(boost::allocator_arg_t, Allocator a)
        {
          typedef typename Allocator::template rebind<detail::future_object<R> >::other A2;
          A2 a2(a);
          typedef thread_detail::allocator_destructor<A2> D;

          future_ = future_ptr(::new(a2.allocate(1)) detail::future_object<R>(), D(a2, 1) );
          future_obtained = false;
        }
#endif
        promise():
#if defined BOOST_THREAD_PROVIDES_PROMISE_LAZY
            future_(),
#else
            future_(new detail::future_object<R>()),
#endif
            future_obtained(false)
        {}

        ~promise()
        {
            if(future_)
            {
                boost::unique_lock<boost::mutex> lock(future_->mutex);

                if(!future_->done)
                {
                    future_->mark_exceptional_finish_internal(boost::copy_exception(broken_promise()), lock);
                }
            }
        }

        // Assignment
        promise(BOOST_THREAD_RV_REF(promise) rhs) BOOST_NOEXCEPT :
            future_(BOOST_THREAD_RV(rhs).future_),future_obtained(BOOST_THREAD_RV(rhs).future_obtained)
        {
            BOOST_THREAD_RV(rhs).future_.reset();
            BOOST_THREAD_RV(rhs).future_obtained=false;
        }
        promise & operator=(BOOST_THREAD_RV_REF(promise) rhs) BOOST_NOEXCEPT
        {
            future_=BOOST_THREAD_RV(rhs).future_;
            future_obtained=BOOST_THREAD_RV(rhs).future_obtained;
            BOOST_THREAD_RV(rhs).future_.reset();
            BOOST_THREAD_RV(rhs).future_obtained=false;
            return *this;
        }

        void swap(promise& other)
        {
            future_.swap(other.future_);
            std::swap(future_obtained,other.future_obtained);
        }

        // Result retrieval
        BOOST_THREAD_FUTURE<R> get_future()
        {
            lazy_init();
            if (future_.get()==0)
            {
                boost::throw_exception(promise_moved());
            }
            if (future_obtained)
            {
                boost::throw_exception(future_already_retrieved());
            }
            future_obtained=true;
            return BOOST_THREAD_FUTURE<R>(future_);
        }

        void set_value(typename detail::future_traits<R>::source_reference_type r)
        {
            lazy_init();
            boost::unique_lock<boost::mutex> lock(future_->mutex);
            if(future_->done)
            {
                boost::throw_exception(promise_already_satisfied());
            }
            future_->mark_finished_with_result_internal(r, lock);
        }

//         void set_value(R && r);
        void set_value(typename detail::future_traits<R>::rvalue_source_type r)
        {
            lazy_init();
            boost::unique_lock<boost::mutex> lock(future_->mutex);
            if(future_->done)
            {
                boost::throw_exception(promise_already_satisfied());
            }
            future_->mark_finished_with_result_internal(static_cast<typename detail::future_traits<R>::rvalue_source_type>(r), lock);
        }

        void set_exception(boost::exception_ptr p)
        {
            lazy_init();
            boost::unique_lock<boost::mutex> lock(future_->mutex);
            if(future_->done)
            {
                boost::throw_exception(promise_already_satisfied());
            }
            future_->mark_exceptional_finish_internal(p, lock);
        }

        // setting the result with deferred notification
        void set_value_at_thread_exit(const R& r)
        {
          if (future_.get()==0)
          {
              boost::throw_exception(promise_moved());
          }
          future_->set_value_at_thread_exit(r);
        }

        void set_value_at_thread_exit(BOOST_THREAD_RV_REF(R) r)
        {
          if (future_.get()==0)
          {
              boost::throw_exception(promise_moved());
          }
          future_->set_value_at_thread_exit(boost::move(r));
        }
        void set_exception_at_thread_exit(exception_ptr e)
        {
          if (future_.get()==0)
          {
              boost::throw_exception(promise_moved());
          }
          future_->set_exception_at_thread_exit(e);
        }

        template<typename F>
        void set_wait_callback(F f)
        {
            lazy_init();
            future_->set_wait_callback(f,this);
        }

    };

    template <typename R>
    class promise<R&>
    {
        typedef boost::shared_ptr<detail::future_object<R&> > future_ptr;

        future_ptr future_;
        bool future_obtained;

        void lazy_init()
        {
#if defined BOOST_THREAD_PROVIDES_PROMISE_LAZY
            if(!atomic_load(&future_))
            {
                future_ptr blank;
                atomic_compare_exchange(&future_,&blank,future_ptr(new detail::future_object<R&>));
            }
#endif
        }

    public:
        BOOST_THREAD_MOVABLE_ONLY(promise)
#if defined BOOST_THREAD_PROVIDES_FUTURE_CTOR_ALLOCATORS
        template <class Allocator>
        promise(boost::allocator_arg_t, Allocator a)
        {
          typedef typename Allocator::template rebind<detail::future_object<R&> >::other A2;
          A2 a2(a);
          typedef thread_detail::allocator_destructor<A2> D;

          future_ = future_ptr(::new(a2.allocate(1)) detail::future_object<R&>(), D(a2, 1) );
          future_obtained = false;
        }
#endif
        promise():
#if defined BOOST_THREAD_PROVIDES_PROMISE_LAZY
            future_(),
#else
            future_(new detail::future_object<R&>()),
#endif
            future_obtained(false)
        {}

        ~promise()
        {
            if(future_)
            {
                boost::unique_lock<boost::mutex> lock(future_->mutex);

                if(!future_->done)
                {
                    future_->mark_exceptional_finish_internal(boost::copy_exception(broken_promise()), lock);
                }
            }
        }

        // Assignment
        promise(BOOST_THREAD_RV_REF(promise) rhs) BOOST_NOEXCEPT :
            future_(BOOST_THREAD_RV(rhs).future_),future_obtained(BOOST_THREAD_RV(rhs).future_obtained)
        {
            BOOST_THREAD_RV(rhs).future_.reset();
            BOOST_THREAD_RV(rhs).future_obtained=false;
        }
        promise & operator=(BOOST_THREAD_RV_REF(promise) rhs) BOOST_NOEXCEPT
        {
            future_=BOOST_THREAD_RV(rhs).future_;
            future_obtained=BOOST_THREAD_RV(rhs).future_obtained;
            BOOST_THREAD_RV(rhs).future_.reset();
            BOOST_THREAD_RV(rhs).future_obtained=false;
            return *this;
        }

        void swap(promise& other)
        {
            future_.swap(other.future_);
            std::swap(future_obtained,other.future_obtained);
        }

        // Result retrieval
        BOOST_THREAD_FUTURE<R&> get_future()
        {
            lazy_init();
            if (future_.get()==0)
            {
                boost::throw_exception(promise_moved());
            }
            if (future_obtained)
            {
                boost::throw_exception(future_already_retrieved());
            }
            future_obtained=true;
            return BOOST_THREAD_FUTURE<R&>(future_);
        }

        void set_value(R& r)
        {
            lazy_init();
            boost::unique_lock<boost::mutex> lock(future_->mutex);
            if(future_->done)
            {
                boost::throw_exception(promise_already_satisfied());
            }
            future_->mark_finished_with_result_internal(r, lock);
        }

        void set_exception(boost::exception_ptr p)
        {
            lazy_init();
            boost::unique_lock<boost::mutex> lock(future_->mutex);
            if(future_->done)
            {
                boost::throw_exception(promise_already_satisfied());
            }
            future_->mark_exceptional_finish_internal(p, lock);
        }

        // setting the result with deferred notification
        void set_value_at_thread_exit(R& r)
        {
          if (future_.get()==0)
          {
              boost::throw_exception(promise_moved());
          }
          future_->set_value_at_thread_exit(r);
        }

        void set_exception_at_thread_exit(exception_ptr e)
        {
          if (future_.get()==0)
          {
              boost::throw_exception(promise_moved());
          }
          future_->set_exception_at_thread_exit(e);
        }

        template<typename F>
        void set_wait_callback(F f)
        {
            lazy_init();
            future_->set_wait_callback(f,this);
        }

    };
    template <>
    class promise<void>
    {
        typedef boost::shared_ptr<detail::future_object<void> > future_ptr;

        future_ptr future_;
        bool future_obtained;

        void lazy_init()
        {
#if defined BOOST_THREAD_PROVIDES_PROMISE_LAZY
            if(!atomic_load(&future_))
            {
                future_ptr blank;
                atomic_compare_exchange(&future_,&blank,future_ptr(new detail::future_object<void>));
            }
#endif
        }
    public:
        BOOST_THREAD_MOVABLE_ONLY(promise)

#if defined BOOST_THREAD_PROVIDES_FUTURE_CTOR_ALLOCATORS
        template <class Allocator>
        promise(boost::allocator_arg_t, Allocator a)
        {
          typedef typename Allocator::template rebind<detail::future_object<void> >::other A2;
          A2 a2(a);
          typedef thread_detail::allocator_destructor<A2> D;

          future_ = future_ptr(::new(a2.allocate(1)) detail::future_object<void>(), D(a2, 1) );
          future_obtained = false;
        }
#endif
        promise():
#if defined BOOST_THREAD_PROVIDES_PROMISE_LAZY
            future_(),
#else
            future_(new detail::future_object<void>),
#endif
            future_obtained(false)
        {}

        ~promise()
        {
            if(future_)
            {
                boost::unique_lock<boost::mutex> lock(future_->mutex);

                if(!future_->done)
                {
                    future_->mark_exceptional_finish_internal(boost::copy_exception(broken_promise()), lock);
                }
            }
        }

        // Assignment
        promise(BOOST_THREAD_RV_REF(promise) rhs) BOOST_NOEXCEPT :
            future_(BOOST_THREAD_RV(rhs).future_),future_obtained(BOOST_THREAD_RV(rhs).future_obtained)
        {
          // we need to release the future as shared_ptr doesn't implements move semantics
            BOOST_THREAD_RV(rhs).future_.reset();
            BOOST_THREAD_RV(rhs).future_obtained=false;
        }

        promise & operator=(BOOST_THREAD_RV_REF(promise) rhs) BOOST_NOEXCEPT
        {
            future_=BOOST_THREAD_RV(rhs).future_;
            future_obtained=BOOST_THREAD_RV(rhs).future_obtained;
            BOOST_THREAD_RV(rhs).future_.reset();
            BOOST_THREAD_RV(rhs).future_obtained=false;
            return *this;
        }

        void swap(promise& other)
        {
            future_.swap(other.future_);
            std::swap(future_obtained,other.future_obtained);
        }

        // Result retrieval
        BOOST_THREAD_FUTURE<void> get_future()
        {
            lazy_init();

            if (future_.get()==0)
            {
                boost::throw_exception(promise_moved());
            }
            if(future_obtained)
            {
                boost::throw_exception(future_already_retrieved());
            }
            future_obtained=true;
            return BOOST_THREAD_FUTURE<void>(future_);
        }

        void set_value()
        {
            lazy_init();
            boost::unique_lock<boost::mutex> lock(future_->mutex);
            if(future_->done)
            {
                boost::throw_exception(promise_already_satisfied());
            }
            future_->mark_finished_with_result_internal(lock);
        }

        void set_exception(boost::exception_ptr p)
        {
            lazy_init();
            boost::unique_lock<boost::mutex> lock(future_->mutex);
            if(future_->done)
            {
                boost::throw_exception(promise_already_satisfied());
            }
            future_->mark_exceptional_finish_internal(p,lock);
        }

        // setting the result with deferred notification
        void set_value_at_thread_exit()
        {
          if (future_.get()==0)
          {
              boost::throw_exception(promise_moved());
          }
          future_->set_value_at_thread_exit();
        }

        void set_exception_at_thread_exit(exception_ptr e)
        {
          if (future_.get()==0)
          {
              boost::throw_exception(promise_moved());
          }
          future_->set_exception_at_thread_exit(e);
        }

        template<typename F>
        void set_wait_callback(F f)
        {
            lazy_init();
            future_->set_wait_callback(f,this);
        }

    };

#if defined BOOST_THREAD_PROVIDES_FUTURE_CTOR_ALLOCATORS
    namespace container
    {
      template <class R, class Alloc>
      struct uses_allocator<promise<R> , Alloc> : true_type
      {
      };
    }
#endif

    BOOST_THREAD_DCL_MOVABLE_BEG(T) promise<T> BOOST_THREAD_DCL_MOVABLE_END

    namespace detail
    {
#if defined BOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK
      template<typename R>
      struct task_base;
#if defined(BOOST_THREAD_PROVIDES_VARIADIC_THREAD)
      template<typename R, typename ...ArgTypes>
      struct task_base<R(ArgTypes...)>:
#else
      template<typename R>
      struct task_base<R()>:
#endif
#else
      template<typename R>
      struct task_base:
#endif
            detail::future_object<R>
        {
            bool started;

            task_base():
                started(false)
            {}

            void reset()
            {
              started=false;
            }
#if defined BOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK && defined(BOOST_THREAD_PROVIDES_VARIADIC_THREAD)
            virtual void do_run(BOOST_THREAD_RV_REF(ArgTypes) ... args)=0;
            void run(BOOST_THREAD_RV_REF(ArgTypes) ... args)
#else
            virtual void do_run()=0;
            void run()
#endif
            {
                {
                    boost::lock_guard<boost::mutex> lk(this->mutex);
                    if(started)
                    {
                        boost::throw_exception(task_already_started());
                    }
                    started=true;
                }
#if defined BOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK && defined(BOOST_THREAD_PROVIDES_VARIADIC_THREAD)
                do_run(boost::forward<ArgTypes>(args)...);
#else
                do_run();
#endif
            }

#if defined BOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK && defined(BOOST_THREAD_PROVIDES_VARIADIC_THREAD)
            virtual void do_apply(BOOST_THREAD_RV_REF(ArgTypes) ... args)=0;
            void apply(BOOST_THREAD_RV_REF(ArgTypes) ... args)
#else
            virtual void do_apply()=0;
            void apply()
#endif
            {
                {
                    boost::lock_guard<boost::mutex> lk(this->mutex);
                    if(started)
                    {
                        boost::throw_exception(task_already_started());
                    }
                    started=true;
                }
#if defined BOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK && defined(BOOST_THREAD_PROVIDES_VARIADIC_THREAD)
                do_apply(boost::forward<ArgTypes>(args)...);
#else
                do_apply();
#endif
            }

            void owner_destroyed()
            {
                boost::unique_lock<boost::mutex> lk(this->mutex);
                if(!started)
                {
                    started=true;
                    this->mark_exceptional_finish_internal(boost::copy_exception(boost::broken_promise()), lk);
                }
            }

        };

#if defined BOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK
        template<typename F, typename R>
        struct task_object;
#if defined(BOOST_THREAD_PROVIDES_VARIADIC_THREAD)
        template<typename F, typename R, typename ...ArgTypes>
        struct task_object<F, R(ArgTypes...)>:
          task_base<R(ArgTypes...)>
#else
        template<typename F, typename R>
        struct task_object<F, R()>:
          task_base<R()>
#endif
#else
        template<typename F, typename R>
        struct task_object:
            task_base<R>
#endif
        {
        private:
          task_object(task_object&);
        public:
            F f;
#ifndef BOOST_NO_CXX11_RVALUE_REFERENCES
            task_object(BOOST_THREAD_RV_REF(F) f_):
              f(boost::forward<F>(f_))
            {}
#else
            task_object(F const& f_):
                f(f_)
            {}
            task_object(BOOST_THREAD_RV_REF(F) f_):
                f(boost::move(f_)) // TODO forward
            {}
#endif

#if defined BOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK && defined(BOOST_THREAD_PROVIDES_VARIADIC_THREAD)
            void do_apply(BOOST_THREAD_RV_REF(ArgTypes) ... args)
            {
                try
                {
                    this->set_value_at_thread_exit(f(boost::forward<ArgTypes>(args)...));
                }
#else
            void do_apply()
            {
                try
                {
                    this->set_value_at_thread_exit(f());
                }
#endif
#if defined BOOST_THREAD_PROVIDES_INTERRUPTIONS
                catch(thread_interrupted& )
                {
                    this->set_interrupted_at_thread_exit();
                }
#endif
                catch(...)
                {
                    this->set_exception_at_thread_exit(current_exception());
                }
            }

#if defined BOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK && defined(BOOST_THREAD_PROVIDES_VARIADIC_THREAD)
            void do_run(BOOST_THREAD_RV_REF(ArgTypes) ... args)
            {
                try
                {
                    this->mark_finished_with_result(f(boost::forward<ArgTypes>(args)...));
                }
#else
            void do_run()
            {
                try
                {
                    this->mark_finished_with_result(f());
                }
#endif
#if defined BOOST_THREAD_PROVIDES_INTERRUPTIONS
                catch(thread_interrupted& )
                {
                    this->mark_interrupted_finish();
                }
#endif
                catch(...)
                {
                    this->mark_exceptional_finish();
                }
            }
        };

#if defined(BOOST_THREAD_RVALUE_REFERENCES_DONT_MATCH_FUNTION_PTR)

#if defined BOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK
#if defined(BOOST_THREAD_PROVIDES_VARIADIC_THREAD)
        template<typename R, typename ...ArgTypes>
        struct task_object<R (*)(ArgTypes...), R(ArgTypes...)>:
          task_base<R(ArgTypes...)>
#else
        template<typename R>
        struct task_object<R (*)(), R()>:
          task_base<R()>
#endif
#else
        template<typename R>
        struct task_object<R (*)(), R> :
           task_base<R>
#endif
            {
            private:
              task_object(task_object&);
            public:
                R (*f)();
                task_object(R (*f_)()):
                    f(f_)
                {}


#if defined BOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK && defined(BOOST_THREAD_PROVIDES_VARIADIC_THREAD)
                void do_apply(BOOST_THREAD_RV_REF(ArgTypes) ... args)
                {
                    try
                    {
                        this->set_value_at_thread_exit(f(boost::forward<ArgTypes>(args)...));
                    }
#else
                void do_apply()
                {
                    try
                    {
                        this->set_value_at_thread_exit(f());
                    }
#endif
#if defined BOOST_THREAD_PROVIDES_INTERRUPTIONS
                    catch(thread_interrupted& )
                    {
                        this->set_interrupted_at_thread_exit();
                    }
#endif
                    catch(...)
                    {
                        this->set_exception_at_thread_exit(current_exception());
                    }
                }


#if defined BOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK && defined(BOOST_THREAD_PROVIDES_VARIADIC_THREAD)
                void do_run(BOOST_THREAD_RV_REF(ArgTypes) ... args)
                {
                    try
                    {
                        this->mark_finished_with_result(f(boost::forward<ArgTypes>(args)...));
                    }
#else
                void do_run()
                {
                    try
                    {
                        this->mark_finished_with_result(f());
                    }
#endif
#if defined BOOST_THREAD_PROVIDES_INTERRUPTIONS
                    catch(thread_interrupted& )
                    {
                        this->mark_interrupted_finish();
                    }
#endif
                    catch(...)
                    {
                        this->mark_exceptional_finish();
                    }
                }
            };
#endif

#if defined BOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK
#if defined(BOOST_THREAD_PROVIDES_VARIADIC_THREAD)
        template<typename F, typename ...ArgTypes>
        struct task_object<F, void(ArgTypes...)>:
          task_base<void(ArgTypes...)>
#else
        template<typename F>
        struct task_object<F, void()>:
          task_base<void()>
#endif
#else
        template<typename F>
        struct task_object<F,void>:
          task_base<void>
#endif
        {
        private:
          task_object(task_object&);
        public:
            F f;
#ifndef BOOST_NO_CXX11_RVALUE_REFERENCES
            task_object(BOOST_THREAD_RV_REF(F) f_):
              f(boost::forward<F>(f_))
            {}
#else
            task_object(F const& f_):
                f(f_)
            {}
            task_object(BOOST_THREAD_RV_REF(F) f_):
                f(boost::move(f_)) // TODO forward
            {}
#endif

#if defined BOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK && defined(BOOST_THREAD_PROVIDES_VARIADIC_THREAD)
            void do_apply(BOOST_THREAD_RV_REF(ArgTypes) ... args)
            {
              try
              {
                f(boost::forward<ArgTypes>(args)...);
#else
            void do_apply()
            {
                try
                {
                    f();
#endif
                  this->set_value_at_thread_exit();
                }
#if defined BOOST_THREAD_PROVIDES_INTERRUPTIONS
                catch(thread_interrupted& )
                {
                    this->set_interrupted_at_thread_exit();
                }
#endif
                catch(...)
                {
                    this->set_exception_at_thread_exit(current_exception());
                }
            }

#if defined BOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK && defined(BOOST_THREAD_PROVIDES_VARIADIC_THREAD)
            void do_run(BOOST_THREAD_RV_REF(ArgTypes) ... args)
            {
                try
                {
                    f(boost::forward<ArgTypes>(args)...);
#else
            void do_run()
            {
                try
                {
                    f();
#endif
                    this->mark_finished_with_result();
                }
#if defined BOOST_THREAD_PROVIDES_INTERRUPTIONS
                catch(thread_interrupted& )
                {
                    this->mark_interrupted_finish();
                }
#endif
                catch(...)
                {
                    this->mark_exceptional_finish();
                }
            }
        };

#if defined BOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK
#if defined(BOOST_THREAD_PROVIDES_VARIADIC_THREAD)
        template<typename ...ArgTypes>
        struct task_object<void (*)(ArgTypes...), void(ArgTypes...)>:
        task_base<void(ArgTypes...)>
#else
        template<>
        struct task_object<void (*)(), void()>:
        task_base<void()>
#endif
#else
        template<>
        struct task_object<void (*)(),void>:
          task_base<void>
#endif
        {
        private:
          task_object(task_object&);
        public:
            void (*f)();
            task_object(void (*f_)()):
                f(f_)
            {}

#if defined BOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK && defined(BOOST_THREAD_PROVIDES_VARIADIC_THREAD)
            void do_apply(BOOST_THREAD_RV_REF(ArgTypes) ... args)
            {
                try
                {
                    f(boost::forward<ArgTypes>(args)...);
#else
            void do_apply()
            {
                try
                {
                    f();
#endif
                    this->set_value_at_thread_exit();
                }
#if defined BOOST_THREAD_PROVIDES_INTERRUPTIONS
                catch(thread_interrupted& )
                {
                    this->set_interrupted_at_thread_exit();
                }
#endif
                catch(...)
                {
                    this->set_exception_at_thread_exit(current_exception());
                }
            }

#if defined BOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK && defined(BOOST_THREAD_PROVIDES_VARIADIC_THREAD)
            void do_run(BOOST_THREAD_RV_REF(ArgTypes) ... args)
            {
                try
                {
                    f(boost::forward<ArgTypes>(args)...);
#else
            void do_run()
            {
                try
                {
                  f();
#endif
                  this->mark_finished_with_result();
                }
#if defined BOOST_THREAD_PROVIDES_INTERRUPTIONS
                catch(thread_interrupted& )
                {
                    this->mark_interrupted_finish();
                }
#endif
                catch(...)
                {
                    this->mark_exceptional_finish();
                }
            }
        };

    }


#if defined BOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK
  #if defined(BOOST_THREAD_PROVIDES_VARIADIC_THREAD)
    template<typename R, typename ...ArgTypes>
    class packaged_task<R(ArgTypes...)>
    {
      typedef boost::shared_ptr<detail::task_base<R(ArgTypes...)> > task_ptr;
      boost::shared_ptr<detail::task_base<R(ArgTypes...)> > task;
  #else
    template<typename R>
    class packaged_task<R()>
    {
      typedef boost::shared_ptr<detail::task_base<R()> > task_ptr;
      boost::shared_ptr<detail::task_base<R()> > task;
  #endif
#else
    template<typename R>
    class packaged_task
    {
      typedef boost::shared_ptr<detail::task_base<R> > task_ptr;
      boost::shared_ptr<detail::task_base<R> > task;
#endif
        bool future_obtained;
        struct dummy;

    public:
        typedef R result_type;
        BOOST_THREAD_MOVABLE_ONLY(packaged_task)

        packaged_task():
            future_obtained(false)
        {}

        // construction and destruction
#if defined(BOOST_THREAD_RVALUE_REFERENCES_DONT_MATCH_FUNTION_PTR)

#if defined BOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK
  #if defined(BOOST_THREAD_PROVIDES_VARIADIC_THREAD)
        explicit packaged_task(R(*f)(), BOOST_THREAD_FWD_REF(ArgTypes)... args)
        {
            typedef R(*FR)(BOOST_THREAD_FWD_REF(ArgTypes)...);
            typedef detail::task_object<FR,R(ArgTypes...)> task_object_type;
            task= task_ptr(new task_object_type(f, boost::forward<ArgTypes>(args)...));
            future_obtained=false;
        }
  #else
        explicit packaged_task(R(*f)())
        {
            typedef R(*FR)();
            typedef detail::task_object<FR,R()> task_object_type;
            task= task_ptr(new task_object_type(f));
            future_obtained=false;
        }
  #endif
#else
        explicit packaged_task(R(*f)())
        {
              typedef R(*FR)();
            typedef detail::task_object<FR,R> task_object_type;
            task= task_ptr(new task_object_type(f));
            future_obtained=false;
        }
#endif
#endif
#ifndef BOOST_NO_CXX11_RVALUE_REFERENCES
        template <class F>
        explicit packaged_task(BOOST_THREAD_RV_REF(F) f
            , typename disable_if<is_same<typename decay<F>::type, packaged_task>, dummy* >::type=0
            )
        {
          //typedef typename remove_cv<typename remove_reference<F>::type>::type FR;
          typedef F FR;
#if defined BOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK
  #if defined(BOOST_THREAD_PROVIDES_VARIADIC_THREAD)
            typedef detail::task_object<FR,R(ArgTypes...)> task_object_type;
  #else
            typedef detail::task_object<FR,R()> task_object_type;
  #endif
#else
            typedef detail::task_object<FR,R> task_object_type;
#endif
            task = task_ptr(new task_object_type(boost::forward<F>(f)));
            future_obtained = false;

        }

#else
        template <class F>
        explicit packaged_task(F const& f
            , typename disable_if<is_same<typename decay<F>::type, packaged_task>, dummy* >::type=0
            )
        {
#if defined BOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK
  #if defined(BOOST_THREAD_PROVIDES_VARIADIC_THREAD)
            typedef detail::task_object<F,R(ArgTypes...)> task_object_type;
  #else
            typedef detail::task_object<F,R()> task_object_type;
  #endif
#else
            typedef detail::task_object<F,R> task_object_type;
#endif
            task = task_ptr(new task_object_type(f));
            future_obtained=false;
        }
        template <class F>
        explicit packaged_task(BOOST_THREAD_RV_REF(F) f)
        {
#if defined BOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK
#if defined(BOOST_THREAD_PROVIDES_VARIADIC_THREAD)
            typedef detail::task_object<F,R(ArgTypes...)> task_object_type;
            task = task_ptr(new task_object_type(boost::forward<F>(f)));
#else
            typedef detail::task_object<F,R()> task_object_type;
            task = task_ptr(new task_object_type(boost::move<F>(f))); // TODO forward
#endif
#else
            typedef detail::task_object<F,R> task_object_type;
            task = task_ptr(new task_object_type(boost::forward<F>(f)));
#endif
            future_obtained=false;

        }
#endif

#if defined BOOST_THREAD_PROVIDES_FUTURE_CTOR_ALLOCATORS
#if defined(BOOST_THREAD_RVALUE_REFERENCES_DONT_MATCH_FUNTION_PTR)
        template <class Allocator>
        packaged_task(boost::allocator_arg_t, Allocator a, R(*f)())
        {
          typedef R(*FR)();
#if defined BOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK
  #if defined(BOOST_THREAD_PROVIDES_VARIADIC_THREAD)
          typedef detail::task_object<FR,R(ArgTypes...)> task_object_type;
  #else
          typedef detail::task_object<FR,R()> task_object_type;
  #endif
#else
          typedef detail::task_object<FR,R> task_object_type;
#endif
          typedef typename Allocator::template rebind<task_object_type>::other A2;
          A2 a2(a);
          typedef thread_detail::allocator_destructor<A2> D;

          task = task_ptr(::new(a2.allocate(1)) task_object_type(f), D(a2, 1) );
          future_obtained = false;
        }
#endif // BOOST_THREAD_RVALUE_REFERENCES_DONT_MATCH_FUNTION_PTR

#if ! defined BOOST_NO_CXX11_RVALUE_REFERENCES
        template <class F, class Allocator>
        packaged_task(boost::allocator_arg_t, Allocator a, BOOST_THREAD_RV_REF(F) f)
        {
          //typedef typename remove_cv<typename remove_reference<F>::type>::type FR;
          typedef F FR;
#if defined BOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK
  #if defined(BOOST_THREAD_PROVIDES_VARIADIC_THREAD)
          typedef detail::task_object<FR,R(ArgTypes...)> task_object_type;
  #else
          typedef detail::task_object<FR,R()> task_object_type;
  #endif
#else
          typedef detail::task_object<FR,R> task_object_type;
#endif
          typedef typename Allocator::template rebind<task_object_type>::other A2;
          A2 a2(a);
          typedef thread_detail::allocator_destructor<A2> D;

          task = task_ptr(::new(a2.allocate(1)) task_object_type(boost::forward<F>(f)), D(a2, 1) );
          future_obtained = false;
        }
#else // ! defined BOOST_NO_CXX11_RVALUE_REFERENCES
        template <class F, class Allocator>
        packaged_task(boost::allocator_arg_t, Allocator a, const F& f)
        {
#if defined BOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK
  #if defined(BOOST_THREAD_PROVIDES_VARIADIC_THREAD)
          typedef detail::task_object<F,R(ArgTypes...)> task_object_type;
  #else
          typedef detail::task_object<F,R()> task_object_type;
  #endif
#else
          typedef detail::task_object<F,R> task_object_type;
#endif
          typedef typename Allocator::template rebind<task_object_type>::other A2;
          A2 a2(a);
          typedef thread_detail::allocator_destructor<A2> D;

          task = task_ptr(::new(a2.allocate(1)) task_object_type(f), D(a2, 1) );
          future_obtained = false;
        }
        template <class F, class Allocator>
        packaged_task(boost::allocator_arg_t, Allocator a, BOOST_THREAD_RV_REF(F) f)
        {
#if defined BOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK
  #if defined(BOOST_THREAD_PROVIDES_VARIADIC_THREAD)
          typedef detail::task_object<F,R(ArgTypes...)> task_object_type;
  #else
          typedef detail::task_object<F,R()> task_object_type;
  #endif
#else
          typedef detail::task_object<F,R> task_object_type;
#endif
          typedef typename Allocator::template rebind<task_object_type>::other A2;
          A2 a2(a);
          typedef thread_detail::allocator_destructor<A2> D;

#if defined BOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK && defined(BOOST_THREAD_PROVIDES_VARIADIC_THREAD)
          task = task_ptr(::new(a2.allocate(1)) task_object_type(boost::forward<F>(f)), D(a2, 1) );
#else
          task = task_ptr(::new(a2.allocate(1)) task_object_type(boost::move(f)), D(a2, 1) );  // TODO forward
#endif
          future_obtained = false;
        }

#endif //BOOST_NO_CXX11_RVALUE_REFERENCES
#endif // BOOST_THREAD_PROVIDES_FUTURE_CTOR_ALLOCATORS

        ~packaged_task()
        {
            if(task)
            {
                task->owner_destroyed();
            }
        }

        // assignment
        packaged_task(BOOST_THREAD_RV_REF(packaged_task) other) BOOST_NOEXCEPT :
            future_obtained(BOOST_THREAD_RV(other).future_obtained)
        {
            task.swap(BOOST_THREAD_RV(other).task);
            BOOST_THREAD_RV(other).future_obtained=false;
        }
        packaged_task& operator=(BOOST_THREAD_RV_REF(packaged_task) other) BOOST_NOEXCEPT
        {

            // todo use forward
            packaged_task temp(static_cast<BOOST_THREAD_RV_REF(packaged_task)>(other));
            swap(temp);
            return *this;
        }

        void reset()
        {
            if (!valid())
                throw future_error(system::make_error_code(future_errc::no_state));
            task->reset();
            future_obtained=false;
        }

        void swap(packaged_task& other) BOOST_NOEXCEPT
        {
            task.swap(other.task);
            std::swap(future_obtained,other.future_obtained);
        }
        bool valid() const BOOST_NOEXCEPT
        {
          return task.get()!=0;
        }

        // result retrieval
        BOOST_THREAD_FUTURE<R> get_future()
        {

            if(!task)
            {
                boost::throw_exception(task_moved());
            }
            else if(!future_obtained)
            {
                future_obtained=true;
                return BOOST_THREAD_FUTURE<R>(task);
            }
            else
            {
                boost::throw_exception(future_already_retrieved());
            }
            //return BOOST_THREAD_FUTURE<R>();

        }


        // execution
#if defined BOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK && defined(BOOST_THREAD_PROVIDES_VARIADIC_THREAD)
        void operator()(BOOST_THREAD_RV_REF(ArgTypes)... args)
        {
            if(!task)
            {
                boost::throw_exception(task_moved());
            }
            task->run(boost::forward<ArgTypes>(args)...);
        }
        void make_ready_at_thread_exit(ArgTypes... args)
        {
          if(!task)
          {
              boost::throw_exception(task_moved());
          }
          if (task->has_value())
          {
                boost::throw_exception(promise_already_satisfied());
          }
          task->apply(boost::forward<ArgTypes>(args)...);
        }
#else
        void operator()()
        {
            if(!task)
            {
                boost::throw_exception(task_moved());
            }
            task->run();
        }
        void make_ready_at_thread_exit()
        {
          if(!task)
          {
              boost::throw_exception(task_moved());
          }
          if (task->has_value())
                boost::throw_exception(promise_already_satisfied());
          task->apply();
        }
#endif
        template<typename F>
        void set_wait_callback(F f)
        {
            task->set_wait_callback(f,this);
        }

    };

#if defined BOOST_THREAD_PROVIDES_FUTURE_CTOR_ALLOCATORS
    namespace container
    {
      template <class R, class Alloc>
      struct uses_allocator<packaged_task<R>, Alloc>
        : public true_type {};
    }
#endif

    BOOST_THREAD_DCL_MOVABLE_BEG(T) packaged_task<T> BOOST_THREAD_DCL_MOVABLE_END

    namespace detail
    {
    ////////////////////////////////
    // make_future_deferred_object
    ////////////////////////////////
    template <class Rp, class Fp>
    BOOST_THREAD_FUTURE<Rp>
    #if ! defined BOOST_NO_CXX11_RVALUE_REFERENCES
    make_future_deferred_object(Fp&& f)
    #else
    make_future_deferred_object(Fp f)
    #endif
    {
      shared_ptr<future_deferred_object<Rp, Fp> >
          h(new future_deferred_object<Rp, Fp>(boost::forward<Fp>(f)));
      return BOOST_THREAD_FUTURE<Rp>(h);
    }

    ////////////////////////////////
    // make_future_async_object
    ////////////////////////////////
    template <class Rp, class Fp>
    BOOST_THREAD_FUTURE<Rp>
    #if ! defined BOOST_NO_CXX11_RVALUE_REFERENCES
    make_future_async_object(Fp&& f)
    #else
    make_future_async_object(Fp f)
    #endif
    {
      shared_ptr<future_async_object<Rp, Fp> >
          h(new future_async_object<Rp, Fp>(boost::forward<Fp>(f)));
      return BOOST_THREAD_FUTURE<Rp>(h);
    }

    }

    ////////////////////////////////
    // template <class F, class... ArgTypes>
    // future<R> async(F&&, ArgTypes&&...);
    ////////////////////////////////

#if defined BOOST_THREAD_RVALUE_REFERENCES_DONT_MATCH_FUNTION_PTR

#if defined BOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK
  #if defined(BOOST_THREAD_PROVIDES_VARIADIC_THREAD)
        template <class R, class... ArgTypes>
        BOOST_THREAD_FUTURE<R>
        async(launch policy, R(*f)(BOOST_THREAD_FWD_REF(ArgTypes)...), BOOST_THREAD_FWD_REF(ArgTypes)... args)
        {
          //typedef packaged_task<R(BOOST_THREAD_FWD_REF(ArgTypes)...)> packaged_task_type;
          typedef packaged_task<R(ArgTypes...)> packaged_task_type;
  #else
        template <class R>
        BOOST_THREAD_FUTURE<R>
        async(launch policy, R(*f)())
        {
          typedef packaged_task<R()> packaged_task_type;
  #endif
#else
        template <class R>
        BOOST_THREAD_FUTURE<R>
        async(launch policy, R(*f)())
        {
          typedef packaged_task<R> packaged_task_type;
#endif
          if (int(policy) & int(launch::async))
            {
              packaged_task_type pt( f );

              BOOST_THREAD_FUTURE<R> ret = pt.get_future();
#if defined BOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK && defined(BOOST_THREAD_PROVIDES_VARIADIC_THREAD)
              boost::thread( boost::move(pt),  boost::forward<ArgTypes>(args)... ).detach();
#else
              boost::thread( boost::move(pt) ).detach();
#endif
              return ::boost::move(ret);
            }
            else if (int(policy) & int(launch::deferred))
            {
              packaged_task_type pt( f );

              BOOST_THREAD_FUTURE<R> ret = pt.get_future();
              return ::boost::move(ret);
            } else {
              BOOST_THREAD_FUTURE<R> ret;
              return ::boost::move(ret);
            }
        }

        template <class R>
        BOOST_THREAD_FUTURE<R>
        async(R(*f)())
        {
            return BOOST_THREAD_MAKE_RV_REF(async(launch(launch::any), f));
        }
#endif

#if defined BOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK
  #if defined(BOOST_THREAD_PROVIDES_VARIADIC_THREAD)

        template <class F, class ...ArgTypes>
        BOOST_THREAD_FUTURE<typename boost::result_of<typename decay<F>::type(
            typename decay<ArgTypes>::type...
        )>::type>
        async(launch policy, BOOST_THREAD_FWD_REF(F) f, BOOST_THREAD_FWD_REF(ArgTypes)... args)
        {

          typedef typename boost::result_of<typename decay<F>::type(
              typename decay<ArgTypes>::type...
          )>::type R;
          typedef packaged_task<R(ArgTypes...)> packaged_task_type;

          typedef detail::async_func<typename decay<F>::type, typename decay<ArgTypes>::type...> BF;
          typedef typename BF::result_type Rp;

  #else
        template <class F>
        BOOST_THREAD_FUTURE<typename boost::result_of<typename decay<F>::type()>::type>
        async(launch policy, BOOST_THREAD_FWD_REF(F)  f)
        {
          typedef typename boost::result_of<typename decay<F>::type()>::type R;
          typedef packaged_task<R()> packaged_task_type;

          typedef detail::async_func<typename decay<F>::type> BF;
          typedef typename BF::result_type Rp;

  #endif
#else
        template <class F>
        BOOST_THREAD_FUTURE<typename boost::result_of<typename decay<F>::type()>::type>
        async(launch policy, BOOST_THREAD_FWD_REF(F)  f)
        {
          typedef typename boost::result_of<typename decay<F>::type()>::type R;
          typedef packaged_task<R> packaged_task_type;

          typedef detail::async_func<typename decay<F>::type> BF;
          typedef typename BF::result_type Rp;
#endif

        if (int(policy) & int(launch::async))
        {
          packaged_task_type pt( boost::forward<F>(f) );

          BOOST_THREAD_FUTURE<R> ret = pt.get_future();
#if defined BOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK && defined(BOOST_THREAD_PROVIDES_VARIADIC_THREAD)
          boost::thread( boost::move(pt), boost::forward<ArgTypes>(args)... ).detach(); // todo forward
#else
          boost::thread( boost::move(pt) ).detach();
#endif
          return ::boost::move(ret);
        }
        else if (int(policy) & int(launch::deferred))
        {
#if defined BOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK && defined(BOOST_THREAD_PROVIDES_VARIADIC_THREAD)
          return boost::detail::make_future_deferred_object<Rp>(
              BF(
                  thread_detail::decay_copy(boost::forward<F>(f))
                  , thread_detail::decay_copy(boost::forward<ArgTypes>(args))...
              )
          );
#else
              BOOST_THREAD_FUTURE<R> ret;
              return ::boost::move(ret);
//          return boost::detail::make_future_deferred_object<Rp>(
//              BF(
//                  thread_detail::decay_copy(boost::forward<F>(f))
//              )
//          );
#endif

        } else {
          BOOST_THREAD_FUTURE<R> ret;
          return ::boost::move(ret);
        }
    }
    template <class F>
    BOOST_THREAD_FUTURE<typename boost::result_of<F()>::type>
    async(BOOST_THREAD_RV_REF(F) f)
    {
        return async(launch(launch::any), boost::forward<F>(f));
    }


  ////////////////////////////////
  // make_future
  ////////////////////////////////
  template <typename T>
  BOOST_THREAD_FUTURE<typename decay<T>::type> make_future(BOOST_THREAD_FWD_REF(T) value)
  {
    typedef typename decay<T>::type future_type;
    promise<future_type> p;
    p.set_value(boost::forward<T>(value));
    return p.get_future();
  }


  inline BOOST_THREAD_FUTURE<void> make_future()
  {
    promise<void> p;
    return p.get_future();

  }

  ////////////////////////////////
  // make_shared_future
  ////////////////////////////////
  template <typename T>
  shared_future<typename decay<T>::type> make_shared_future(BOOST_THREAD_FWD_REF(T) value)
  {
    typedef typename decay<T>::type future_type;
    promise<future_type> p;
    p.set_value(boost::forward<T>(value));
    return p.get_future().share();
  }


  inline shared_future<void> make_shared_future()
  {
    promise<void> p;
    return p.get_future().share();

  }

  ////////////////////////////////
  // detail::future_continuation
  ////////////////////////////////
#if defined BOOST_THREAD_PROVIDES_FUTURE_CONTINUATION
  namespace detail
  {
      template <typename F, typename R, typename C>
      struct future_continuation : future_continuation_base
      {
        F& parent;
        C continuation;
        launch policy_;
        promise<R> next;

        future_continuation(F& f, BOOST_THREAD_FWD_REF(C) c) :
          parent(f),
          continuation(boost::forward<C>(c)),
          policy_(f.launch_policy()),
          next()
        {}
        future_continuation(F& f, BOOST_THREAD_FWD_REF(C) c, launch policy) :
          parent(f),
          continuation(boost::forward<C>(c)),
          policy_(policy),
          next()
        {}
        ~future_continuation()
        {}

        void do_continuation(boost::unique_lock<boost::mutex>& lk)
        {
          try
          {
            lk.unlock();
            // fixme what to do depending on inherits_launch_policy_ and policy_?
//            if (int(policy_) & int(launch::deferred))
            {
              R val = continuation(parent);
              next.set_value(boost::move(val));
            }
//            else
//            {
//              BOOST_THREAD_FUTURE<R> f = async(policy_, continuation, boost::ref(parent));
//              R val = f.get();
//              next.set_value(boost::move(val));
//            }
          }
          catch (...)
          {
            next.set_exception(boost::current_exception());
          }
        }
      private:

        future_continuation(future_continuation const&);
        future_continuation& operator=(future_continuation const&);
      };
#if defined(BOOST_THREAD_RVALUE_REFERENCES_DONT_MATCH_FUNTION_PTR)
      template <typename F, typename R, typename CR>
      struct future_continuation<F,R,CR(*)(F&)> : future_continuation_base
      {
        F& parent;
        CR(*continuation)(F&) ;
        launch policy_;
        promise<R> next;

        future_continuation(F& f, CR(*c)(F&)) :
          parent(f),
          continuation(c),
          policy_(f.launch_policy()),
          next()
        {}
        future_continuation(F& f, CR(*c)(F&), launch policy) :
          parent(f),
          continuation(c),
          policy_(policy),
          next()
        {}
        ~future_continuation()
        {}

        void do_continuation(boost::unique_lock<boost::mutex>& lk)
        {
          try
          {
            lk.unlock();
            // fixme what to do depending on inherits_launch_policy_ and policy_?
//            if (int(policy_) & int(launch::deferred))
            {
              R val = continuation(parent);
              next.set_value(boost::move(val));
            }
//            else
//            {
//              BOOST_THREAD_FUTURE<R> f = async(policy_, continuation, boost::ref(parent));
//              R val = f.get();
//              next.set_value(boost::move(val));
//            }
          }
          catch (...)
          {
            next.set_exception(boost::current_exception());
          }
        }
      private:

        future_continuation(future_continuation const&);
        future_continuation& operator=(future_continuation const&);
      };
#endif
  }

  ////////////////////////////////
  // template<typename F>
  // auto future<R>::then(F&& func) -> BOOST_THREAD_FUTURE<decltype(func(*this))>;
  ////////////////////////////////

  template <typename R>
  template <typename F>
  inline BOOST_THREAD_FUTURE<typename boost::result_of<F(BOOST_THREAD_FUTURE<R>&)>::type>
  BOOST_THREAD_FUTURE<R>::then(launch policy, BOOST_THREAD_RV_REF(F) func)
  {

    typedef typename boost::result_of<F(BOOST_THREAD_FUTURE<R>&)>::type future_type;

    if (this->future_)
    {
      boost::unique_lock<boost::mutex> lock(this->future_->mutex);
      detail::future_continuation<BOOST_THREAD_FUTURE<R>, future_type, F > *ptr =
          new detail::future_continuation<BOOST_THREAD_FUTURE<R>, future_type, F>(*this, boost::forward<F>(func), policy);
      if (ptr==0)
      {
        return BOOST_THREAD_FUTURE<future_type>();
      }
      this->future_->set_continuation_ptr(ptr, lock);
      return ptr->next.get_future();
    }
    else
    {
      // fixme what to do when the future has no associated state?
      return BOOST_THREAD_FUTURE<future_type>();
    }

  }
  template <typename R>
  template <typename F>
  inline BOOST_THREAD_FUTURE<typename boost::result_of<F(BOOST_THREAD_FUTURE<R>&)>::type>
  BOOST_THREAD_FUTURE<R>::then(BOOST_THREAD_RV_REF(F) func)
  {

    typedef typename boost::result_of<F(BOOST_THREAD_FUTURE<R>&)>::type future_type;

    if (this->future_)
    {
      boost::unique_lock<boost::mutex> lock(this->future_->mutex);
      detail::future_continuation<BOOST_THREAD_FUTURE<R>, future_type, F > *ptr =
          new detail::future_continuation<BOOST_THREAD_FUTURE<R>, future_type, F>(*this, boost::forward<F>(func));
      if (ptr==0)
      {
        return BOOST_THREAD_FUTURE<future_type>();
      }
      this->future_->set_continuation_ptr(ptr, lock);
      return ptr->next.get_future();
    } else {
      // fixme what to do when the future has no associated state?
      return BOOST_THREAD_FUTURE<future_type>();
    }

  }
#if defined(BOOST_THREAD_RVALUE_REFERENCES_DONT_MATCH_FUNTION_PTR)
  template <typename R>
  template<typename RF>
  BOOST_THREAD_FUTURE<RF>
  BOOST_THREAD_FUTURE<R>::then(RF(*func)(BOOST_THREAD_FUTURE<R>&))
  {

    typedef RF future_type;

    if (this->future_)
    {
      boost::unique_lock<boost::mutex> lock(this->future_->mutex);
      detail::future_continuation<BOOST_THREAD_FUTURE<R>, future_type, RF(*)(BOOST_THREAD_FUTURE&) > *ptr =
          new detail::future_continuation<BOOST_THREAD_FUTURE<R>, future_type, RF(*)(BOOST_THREAD_FUTURE&)>(*this, func);
      if (ptr==0)
      {
        return BOOST_THREAD_FUTURE<future_type>();
      }
      this->future_->set_continuation_ptr(ptr, lock);
      return ptr->next.get_future();
    } else {
      // fixme what to do when the future has no associated state?
      return BOOST_THREAD_FUTURE<future_type>();
    }

  }
  template <typename R>
  template<typename RF>
  BOOST_THREAD_FUTURE<RF>
  BOOST_THREAD_FUTURE<R>::then(launch policy, RF(*func)(BOOST_THREAD_FUTURE<R>&))
  {

    typedef RF future_type;

    if (this->future_)
    {
      boost::unique_lock<boost::mutex> lock(this->future_->mutex);
      detail::future_continuation<BOOST_THREAD_FUTURE<R>, future_type, RF(*)(BOOST_THREAD_FUTURE&) > *ptr =
          new detail::future_continuation<BOOST_THREAD_FUTURE<R>, future_type, RF(*)(BOOST_THREAD_FUTURE&)>(*this, func, policy);
      if (ptr==0)
      {
        return BOOST_THREAD_FUTURE<future_type>();
      }
      this->future_->set_continuation_ptr(ptr, lock);
      return ptr->next.get_future();
    } else {
      // fixme what to do when the future has no associated state?
      return BOOST_THREAD_FUTURE<future_type>();
    }

  }
#endif

#endif

}

#endif // BOOST_NO_EXCEPTION
#endif // header
