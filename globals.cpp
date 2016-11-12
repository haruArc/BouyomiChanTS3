#include "globals.h"
#include <QtCore/QSettings>

namespace Globals {
    TS3Functions ts3Functions;

    bool sendToMyself = DEFAULTCONFIG_SENDTOMYSELF;
    bool textMessageTargetModeClient = DEFAULTCONFIG_TEXTMESSAGETARGETMODECLIENT;
    bool textMessageTargetModeChannel = DEFAULTCONFIG_TEXTMESSAGETARGETMODECHANNEL;
    bool textMessageTargetModeServer = DEFAULTCONFIG_TEXTMESSAGETARGETMODESERVER;

    void loadConfig() {
        QSettings cfg(QString::fromStdString(getConfigFilePath()), QSettings::IniFormat);
        sendToMyself = cfg.value("sendToMyself", DEFAULTCONFIG_SENDTOMYSELF).toBool();
        textMessageTargetModeClient = cfg.value("textMessageTargetModeClient", DEFAULTCONFIG_TEXTMESSAGETARGETMODECLIENT).toBool();
        textMessageTargetModeChannel = cfg.value("textMessageTargetModeChannel", DEFAULTCONFIG_TEXTMESSAGETARGETMODECHANNEL).toBool();
        textMessageTargetModeServer = cfg.value("textMessageTargetModeServer", DEFAULTCONFIG_TEXTMESSAGETARGETMODESERVER).toBool();
    }

    std::string getConfigFilePath() {
        char* configPath = (char*)malloc(512);
        ts3Functions.getConfigPath(configPath, 512);
        std::string path = configPath;
        free(configPath);
        path.append("Bouyomi.ini");
        return path;
    }

    void getServerConnectionHandlerList(uint64** result)
    {
        ts3Functions.getServerConnectionHandlerList(result);
    }
    unsigned int getServerName(uint64 serverConnectionHandlerID, char** result)
    {
        ts3Functions.getServerVariableAsString(serverConnectionHandlerID, VirtualServerProperties::VIRTUALSERVER_NAME, result);
        return 0;
    }
    unsigned int getServerUniqueIdentifier(uint64 serverConnectionHandlerID, char** result)
    {
        ts3Functions.getServerVariableAsString(serverConnectionHandlerID, VirtualServerProperties::VIRTUALSERVER_UNIQUE_IDENTIFIER, result);
        return 0;
    }
}
