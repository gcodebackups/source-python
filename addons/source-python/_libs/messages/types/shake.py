# ../_libs/messages/type/shake.py

# =============================================================================
# >> IMPORTS
# =============================================================================
# Source.Python Imports
#   Core
from core import GameEngine
#   Messages
from messages.base import BaseMessageNoText
from messages.base import get_usermsg_instance


# =============================================================================
# >> CLASSES
# =============================================================================
class Shake(BaseMessageNoText):
    '''Class used to send Shake messages'''

    def __init__(self, magnitude, time, users=()):
        '''Initializes the class instance and stores the given values'''

        # Store all the base attributes
        self.magnitude = magnitude
        self.time = time
        self.users = users

    def _send_message(self, recipients):
        '''Sends the message to the given recipients'''

        # Create the UserMessage
        UserMessage = get_usermsg_instance(recipients, self._message_index)

        # Write 0 to the UserMessage
        UserMessage.WriteByte(0)

        # Write the magnitude to the UserMessage
        UserMessage.WriteFloat(self.magnitude)

        # Write 1.0 to the UserMessage
        UserMessage.WriteFloat(1.0)

        # Write the time to the UserMessage
        UserMessage.WriteFloat(self.time)

        # Send the message and clean up
        GameEngine.MessageEnd()