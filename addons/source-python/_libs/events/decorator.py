# ../_libs/events/decorator.py

# =============================================================================
# >> IMPORTS
# =============================================================================
# Source.Python Imports
#   Events
from events.manager import EventRegistry


# =============================================================================
# >> CLASSES
# =============================================================================
class event(object):
    '''Event decorator class'''

    def __init__(self, callback):
        '''Store the callback and register the event'''

        # Is the callback callable?
        if not callable(callback):

            # Raise an error
            raise TypeError(
                "'" + type(callback).__name__ + "' object is not callable")

        # Store the callback
        self.callback = callback

        # Register the event
        EventRegistry.RegisterForEvent(self.callback.__name__, self.callback)

    def __call__(self, game_event):
        '''Calls the callback for the event'''

        # Call the callback
        return self.callback(game_event)

    def __del__(self):
        '''Called when the object is deleted from memory'''

        # Unregister the event
        EventRegistry.UnregisterForEvent(self.callback.__name__, self.callback)

        # Delete the object from memory
        super(event, self).__del__()
