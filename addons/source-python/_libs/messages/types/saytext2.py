# ../_libs/messages/types/saytext2.py

# =============================================================================
# >> IMPORTS
# =============================================================================
# Source.Python Imports
#   Core
from core import GameEngine
#   Messages
from messages.base import BaseMessage
from messages.base import get_usermsg_instance


# =============================================================================
# >> GLOBAL VARIABLES
# =============================================================================
# Store starting hex characters that should be at the start of each message
_HexChars = ('\x01', '\x02', '\x03', '\x04', '\x05', '\x06', '\x07')


# =============================================================================
# >> CLASSES
# =============================================================================
class SayText2(BaseMessage):
    '''Class used to send SayText2 messages'''

    def __init__(self, message, index=0, users=(), tokens={}):
        '''Initializes the class instance and stores the given values'''

        # Store all the base attributes
        self.message = message
        self.index = index
        self.users = users
        self.tokens = tokens

    def _send_message(self, recipients, message):
        '''Sends the message to the given recipients'''

        # Create the UserMessage
        UserMessage = get_usermsg_instance(recipients, self._message_index)

        # Write the index to the UserMessage
        UserMessage.WriteByte(self.index)

        # Write 1 to the UserMessage
        UserMessage.WriteByte(1)

        # Does the message start with a hex character?
        if not message.startswith(_HexChars):

            # Add \x01 to the start of the message
            message = '\x01' + self.message

        # Write the message to the UserMessage
        UserMessage.WriteString(message)

        # Send the message to the recipients
        GameEngine.MessageEnd()