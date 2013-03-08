# ../_libs/events/listener.py

# =============================================================================
# >> IMPORTS
# =============================================================================
# Python Imports
#   Sys
import sys

# Source.Python Imports
from Source import event_c
from excepthooks import ExceptHooks


# =============================================================================
# >> CLASSES
# =============================================================================
class _EventListener(list):
    '''Stores callbacks for the given event'''

    def __init__(self, event):
        '''Called when an instance is created'''

        # Store the event name
        self.event = event

        # Store the listener instance
        self.listener = event_c.CGameEventListener()

        # Set the listener's fire_game_event method to the instance's method
        self.listener.fire_game_event = self.fire_game_event

    def append(self, callback):
        '''Adds the callback to the list'''

        # Is the callback already in the list?
        if callback in self:

            # Raise an error
            raise ValueError(
                'Event callback "%s" is already ' % callback +
                'registered for event "%s"' % self.event)

        # Add the callback to the list
        super(_EventListener, self).append(callback)

    def remove(self, callback):
        '''Removes the callback from the event's list'''

        # Is the callback in the list?
        if not callback in self:

            # Raise an error
            raise ValueError(
                'Event callback "%s" is not ' % callback +
                'registered for the event "%s"' % self.event)

        # Remove the callback from the list
        super(_EventListener, self).remove(callback)

    def fire_game_event(self, game_event):
        '''Loops through all callbacks for an event and calls them'''

        # Loop through each callback in the event's list
        for callback in self:

            # Try to call the callback
            try:

                # Call the callback
                callback(game_event)

            # Was an error encountered?
            except:

                # Get the error
                error = sys.exc_info()

                # Print the exception to the console
                ExceptHooks.print_exception(*error)
