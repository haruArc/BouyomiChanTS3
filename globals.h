
#include <string>
#include "ts3_functions.h"

#define DEFAULTCONFIG_SENDTOMYSELF false
#define DEFAULTCONFIG_TEXTMESSAGETARGETMODECLIENT true
#define DEFAULTCONFIG_TEXTMESSAGETARGETMODECHANNEL true
#define DEFAULTCONFIG_TEXTMESSAGETARGETMODESERVER true

namespace Globals {
    extern TS3Functions ts3Functions;

    extern bool sendToMyself;
    extern bool textMessageTargetModeClient;
    extern bool textMessageTargetModeChannel;
    extern bool textMessageTargetModeServer;
    std::string getConfigFilePath();
    void getServerConnectionHandlerList(uint64**  );

    unsigned int getServerName(uint64 serverConnectionHandlerID, char** result);
    unsigned int getServerUniqueIdentifier(uint64 serverConnectionHandlerID, char** result);
}
