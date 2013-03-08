# ../_libs/messages/types/fade.py

# =============================================================================
# >> IMPORTS
# =============================================================================
# Source.Python Imports
from core import GameEngine
#   Messages
from messages.base import BaseMessageNoText
from messages.base import MessageTypes


# =============================================================================
# >> CLASSES
# =============================================================================
class Fade(BaseMessageNoText):
    '''Class used to send Fade messages'''

    def __init__(
            self, fade_type, fade_time, hold_time,
            red, green, blue, alpha=255, users=()):
        '''Initializes the class instance and stores the given values'''

        # Store all the base attributes
        self.fade_type = fade_type
        self.fade_time = fade_time
        self.hold_time = hold_time
        self.red = red
        self.green = green
        self.blue = blue
        self.alpha = alpha
        self.users = users

    def _send_message(self, recipients):
        '''Sends the message to the given recipients'''

        # Create the UserMessage
        UserMessage = self._get_usermsg_instance(recipients)

        # Write the fade time to the UserMessage
        UserMessage.WriteShort(self.fade_time)

        # Write the hold time to the UserMessage
        UserMessage.WriteShort(self.hold_time)

        # Write the fade type to the UserMessage
        UserMessage.WriteShort(self.fade_type)

        # Write the red value to the UserMessage
        UserMessage.WriteByte(self.red)

        # Write the green value to the UserMessage
        UserMessage.WriteByte(self.green)

        # Write the blue value to the UserMessage
        UserMessage.WriteByte(self.blue)

        # Write the alpha value to the UserMessage
        UserMessage.WriteByte(self.alpha)

        # Send the message and clean up
        GameEngine.MessageEnd()

    def _send_protobuf_message(self, recipients):
        '''Sends a protobuf message to the given recipients'''

        # Get the usermessage instance
        UserMessage = self._get_protobuf_instance()

        # Set the message's fade time
        UserMessage.set_duration(self.fade_time)

        # Set the message's hold time
        UserMessage.set_hold_time(self.hold_time)

        # Set the message's fade type
        UserMessage.set_flags(self.fade_type)

        # Set the message's red value
        UserMessage.clr.set_r(self.red)

        # Set the message's green value
        UserMessage.clr.set_g(self.green)

        # Set the message's blue value
        UserMessage.clr.set_b(self.blue)

        # Set the message's alpha value
        UserMessage.clr.set_a(self.alpha)

        # Send the message
        GameEngine.SendUserMessage(
            recipients, MessageTypes[self.__class__.__name__], UserMessage)
