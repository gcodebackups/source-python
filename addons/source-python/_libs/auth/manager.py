# ../_libs/auth/manager.py

# =============================================================================
# >> IMPORTS
# =============================================================================
# Source.Python imports
from public import public
#   Auth
from auth import AuthLogger
from auth.paths import AUTH_PROVIDER_PATH
#   Players
from players.helpers import uniqueid_from_playerinfo
#   Translations
from translations.strings import LangStrings


# =============================================================================
# >> GLOBAL VARIABLES
# =============================================================================
# Get the auth language strings
_auth_strings = LangStrings('_core/auth_strings')

# Get the sp.auth.manager logger
AuthManagerLogger = AuthLogger.manager


# =============================================================================
# >> CLASSES
# =============================================================================
@public
class _AuthManager(dict):
    '''Class used to store loaded auth providers
        and check if a player is authorized'''

    def _load_auth(self, provider):
        '''Loads the given provider'''

        # Send a message that the auth provider is being loaded
        AuthManagerLogger.log_message(
            '[SP Auth] ' + _auth_strings[
                'Loading'].get_string(provider=provider))

        # Is the provider loaded?
        if provider in self:

            # If so, send a message that the provider is already loaded
            AuthManagerLogger.log_message(
                '[SP Auth] ' + _auth_strings[
                    'Already Loaded'].get_string(provider=provider))

            # No need to go further
            return

        # Does the provider's file exist?
        if not AUTH_PROVIDER_PATH.joinpath(provider + '.py').isfile():

            # Send a message that the file does not exist
            AuthManagerLogger.log_message(
                '[SP Auth] ' + _auth_strings[
                    'No Module'].get_string(provider=provider))

            # No need to go further
            return

        # Attempt to load the provider
        self[provider] = __import__(
            'auth.providers.{0}'.format(provider), fromlist=[''])

        # Call the provider's load function
        self[provider].load()

        # Send a message that the provider was loaded
        AuthManagerLogger.log_message(
            '[SP Auth] ' + _auth_strings[
                'Load Successful'].get_string(provider=provider))

    def _unload_auth(self, provider):
        '''Unloads the given provider'''

        # Send a message that the auth provider is being unloaded
        AuthManagerLogger.log_message(
            '[SP Auth] ' + _auth_strings[
                'Unloading'].get_string(provider=provider))

        # Is the provider loaded?
        if not provider in self:

            # If not, send a message that the provider is not loaded
            AuthManagerLogger.log_message(
                '[SP Auth] ' + _auth_strings[
                    'Not Loaded'].get_string(provider=provider))

            # No need to go further
            return

        # Call the providers unload method
        self[provider].unload()

        # Remove the provider
        del self[provider]

        # Send a message that the provider was unloaded
        AuthManagerLogger.log_message(
            '[SP Auth] ' + _auth_strings[
                'Unload Successful'].get_string(provider=provider))

    def _reload_auth(self, provider):
        '''Reloads the given provider'''

        # Unload the provider
        self.unload_auth(provider)

        # Load the provider
        self.load_auth(provider)

    def is_player_authorized(
            self, playerinfo, level=None, permission=None, flag=None):
        '''Checks to see if the player is authorized'''

        # Get the player's uniqueid
        uniqueid = uniqueid_from_playerinfo(playerinfo)

        # Loop through all loaded auth providers
        for provider in self:

            # Is the player authorized?
            if self[provider].is_player_authorized(
                    uniqueid, level, permission, flag):

                # If the player is authorized, return true
                return True

        # If the player is not found authorized for any provider, return false
        return False

# Get the _AuthManager instance
AuthManager = _AuthManager()
