/*
 * TeamSpeak 3 demo plugin
 *
 * Copyright (c) 2008-2014 TeamSpeak Systems GmbH
 */

#ifdef _WIN32
#pragma warning (disable : 4100)  /* Disable Unreferenced parameter warning */
#include <Windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "teamspeak/public_errors.h"
#include "teamspeak/public_errors_rare.h"
#include "teamspeak/public_definitions.h"
#include "teamspeak/public_rare_definitions.h"
#include "teamspeak/clientlib_publicdefinitions.h"
#include "ts3_functions.h"
#include "plugin.h"
#include "globals.h"
#include "dialog.h"
#include <QTcpSocket>

#include <QTranslator>
#include <QLocale>
#include <QApplication>
#include <QSqlDatabase>
#include <QSqlQuery>
using namespace Globals;

#ifdef _WIN32
#define _strcpy(dest, destSize, src) strcpy_s(dest, destSize, src)
#define snprintf sprintf_s
#else
#define _strcpy(dest, destSize, src) { strncpy(dest, src, destSize-1); (dest)[destSize-1] = '\0'; }
#endif

#define PLUGIN_API_VERSION 21
#define INFODATA_BUFSIZE 128

static char* pluginID = NULL;
QHash<uint64, int> hash;
QTranslator translator;

#ifdef _WIN32
/* Helper function to convert wchar_T to Utf-8 encoded strings on Windows */
static int wcharToUtf8(const wchar_t* str, char** result) {
	int outlen = WideCharToMultiByte(CP_UTF8, 0, str, -1, 0, 0, 0, 0);
	*result = (char*)malloc(outlen);
	if(WideCharToMultiByte(CP_UTF8, 0, str, -1, *result, outlen, 0, 0) == 0) {
		*result = NULL;
		return -1;
	}
	return 0;
}
#endif

/*********************************** Required functions ************************************/
/*
 * If any of these required functions is not implemented, TS3 will refuse to load the plugin
 */

/* Unique name identifying this plugin */
const char* ts3plugin_name() {
#ifdef _WIN32
	/* TeamSpeak expects UTF-8 encoded characters. Following demonstrates a possibility how to convert UTF-16 wchar_t into UTF-8. */
	static char* result = NULL;  /* Static variable so it's allocated only once */
	if(!result) {
        const wchar_t* name = L"BouyomiChan";
		if(wcharToUtf8(name, &result) == -1) {  /* Convert name into UTF-8 encoded result */
            strcpy(result, "BouyomiChan");  /* Conversion failed, fallback here */
		}
	}
	return result;
#else
	return "BouyomiChan";
#endif
}

/* Plugin version */
const char* ts3plugin_version() {
    return "2.0.0";
}

/* Plugin API version. Must be the same as the clients API major version, else the plugin fails to load. */
int ts3plugin_apiVersion() {
	return PLUGIN_API_VERSION;
}

/* Plugin author */
const char* ts3plugin_author() {
	/* If you want to use wchar_t, see ts3plugin_name() on how to use */
    return "haru_arc";
}

/* Plugin description */
const char* ts3plugin_description() {
	/* If you want to use wchar_t, see ts3plugin_name() on how to use */
    return "This plugin is to make BouyomiChan  read aloud text message.The BouyomiChan is a Japanese Text-to-Speech software.";
}

/* Set TeamSpeak 3 callback functions */
void ts3plugin_setFunctionPointers(const struct TS3Functions funcs) {
    ts3Functions = funcs;
}

/*
 * Custom code called right after loading the plugin. Returns 0 on success, 1 on failure.
 * If the function returns 1 on failure, the plugin will be unloaded again.
 */
int ts3plugin_init() {

    char* configPath = (char*)malloc(512);
    ts3Functions.getConfigPath(configPath, 512);
    std::string path = configPath;
    path.append("settings.db");

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(QString().fromStdString(path));
    db.open();
    QSqlQuery query;
    query.exec("select value from application where key = 'Language' limit 1;");
    query.next();
    QString currentLang = query.value("value").toString();
    db.close();

    if (currentLang == "")
    {
        currentLang = QLocale::system().name();
    }

    path = configPath;
    path.append("translations");
    free(configPath);

    translator.load(QLatin1String("BouyomiChanTS3_" )+currentLang, QString().fromStdString(path));


    return 0;  /* 0 = success, 1 = failure, -2 = failure but client will not show a "failed to load" warning */
	/* -2 is a very special case and should only be used if a plugin displays a dialog (e.g. overlay) asking the user to disable
	 * the plugin again, avoiding the show another dialog by the client telling the user the plugin failed to load.
	 * For normal case, if a plugin really failed to load because of an error, the correct return value is 1. */
}

/* Custom code called right before the plugin is unloaded */
void ts3plugin_shutdown() {
	/*
	 * Note:
	 * If your plugin implements a settings dialog, it must be closed and deleted here, else the
	 * TeamSpeak client will most likely crash (DLL removed but dialog from DLL code still open).
	 */

	/* Free pluginID if we registered it */
	if(pluginID) {
		free(pluginID);
		pluginID = NULL;
	}
}

/****************************** Optional functions ********************************/
/*
 * Following functions are optional, if not needed you don't need to implement them.
 */

/* Tell client if plugin offers a configuration window. If this function is not implemented, it's an assumed "does not offer" (PLUGIN_OFFERS_NO_CONFIGURE). */
int ts3plugin_offersConfigure() {
	/*
	 * Return values:
	 * PLUGIN_OFFERS_NO_CONFIGURE         - Plugin does not implement ts3plugin_configure
	 * PLUGIN_OFFERS_CONFIGURE_NEW_THREAD - Plugin does implement ts3plugin_configure and requests to run this function in an own thread
	 * PLUGIN_OFFERS_CONFIGURE_QT_THREAD  - Plugin does implement ts3plugin_configure and requests to run this function in the Qt GUI thread
	 */
    return PLUGIN_OFFERS_CONFIGURE_QT_THREAD;  /* In this case ts3plugin_configure does not need to be implemented */
}

/* Plugin might offer a configuration window. If ts3plugin_offersConfigure returns 0, this function does not need to be implemented. */
void ts3plugin_configure(void* handle, void* qParentWidget) {
    QApplication::instance()->installTranslator(&translator);
    BouyomiConfigDialog qConfigDialog((QWidget*)qParentWidget);
    qConfigDialog.exec();
}

/*
 * If the plugin wants to use error return codes, plugin commands, hotkeys or menu items, it needs to register a command ID. This function will be
 * automatically called after the plugin was initialized. This function is optional. If you don't use these features, this function can be omitted.
 * Note the passed pluginID parameter is no longer valid after calling this function, so you must copy it and store it in the plugin.
 */
void ts3plugin_registerPluginID(const char* id) {
    const size_t sz = strlen(id) + 1;
    pluginID = (char*)malloc(sz * sizeof(char));
    _strcpy(pluginID, sz, id);  /* The id buffer will invalidate after exiting this function */
}

/* Plugin command keyword. Return NULL or "" if not used. */
const char* ts3plugin_commandKeyword() {

    return NULL;
}

/* Plugin processes console command. Return 0 if plugin handled the command, 1 if not handled. */
int ts3plugin_processCommand(uint64 serverConnectionHandlerID, const char* command) {

    return 1;  /* Plugin handled command */
}

/*
 * Menu IDs for this plugin. Pass these IDs when creating a menuitem to the TS3 client. When the menu item is triggered,
 * ts3plugin_onMenuItemEvent will be called passing the menu ID of the triggered menu item.
 * These IDs are freely choosable by the plugin author. It's not really needed to use an enum, it just looks prettier.
 */
enum {
    MENU_ID_GLOBAL_1 = 1,
    MENU_ID_GLOBAL_2,
    MENU_ID_GLOBAL_3
};

/* Client changed current server connection handler */
void ts3plugin_currentServerConnectionChanged(uint64 serverConnectionHandlerID) {
    if(!hash.contains(serverConnectionHandlerID))
    {
        hash[serverConnectionHandlerID] = 1;
    }
    if(hash[serverConnectionHandlerID] == 1)
    {
        ts3Functions.setPluginMenuEnabled(pluginID, MENU_ID_GLOBAL_2, 0);
        ts3Functions.setPluginMenuEnabled(pluginID, MENU_ID_GLOBAL_3, 1);
    }
    else
    {
        ts3Functions.setPluginMenuEnabled(pluginID, MENU_ID_GLOBAL_2, 1);
        ts3Functions.setPluginMenuEnabled(pluginID, MENU_ID_GLOBAL_3, 0);
    }
}

/*
 * Implement the following three functions when the plugin should display a line in the server/channel/client info.
 * If any of ts3plugin_infoTitle, ts3plugin_infoData or ts3plugin_freeMemory is missing, the info text will not be displayed.
 */

/* Static title shown in the left column in the info frame */
const char* ts3plugin_infoTitle() {
	return "BouyomiChanTS3";
}

/*
 * Dynamic content shown in the right column in the info frame. Memory for the data string needs to be allocated in this
 * function. The client will call ts3plugin_freeMemory once done with the string to release the allocated memory again.
 * Check the parameter "type" if you want to implement this feature only for specific item types. Set the parameter
 * "data" to NULL to have the client ignore the info data.
 */
void ts3plugin_infoData(uint64 serverConnectionHandlerID, uint64 id, enum PluginItemType type, char** data) {
    if( type == PLUGIN_SERVER)
    {
        *data = (char*)malloc(INFODATA_BUFSIZE * sizeof(char));  /* Must be allocated in the plugin! */

        QApplication::instance()->installTranslator(&translator);

        if(hash[serverConnectionHandlerID] == 1)
        {
            _strcpy(*data, INFODATA_BUFSIZE, (const char*)(QObject::tr("Enabled").toUtf8().constData()));
        }
        else
        {
            _strcpy(*data, INFODATA_BUFSIZE, (const char*)(QObject::tr("Disabled").toUtf8().constData()));
        }
    }
    else
    {
        data = NULL;
    }
}

/* Required to release the memory for parameter "data" allocated in ts3plugin_infoData and ts3plugin_initMenus */
void ts3plugin_freeMemory(void* data) {
    free(data);
}

/*
 * Plugin requests to be always automatically loaded by the TeamSpeak 3 client unless
 * the user manually disabled it in the plugin dialog.
 * This function is optional. If missing, no autoload is assumed.
 */
int ts3plugin_requestAutoload() {
    return 0;  /* 1 = request autoloaded, 0 = do not request autoload */
}

/* Helper function to create a menu item */
static struct PluginMenuItem* createMenuItem(enum PluginMenuType type, int id, const char* text, const char* icon) {
    struct PluginMenuItem* menuItem = (struct PluginMenuItem*)malloc(sizeof(struct PluginMenuItem));
    menuItem->type = type;
    menuItem->id = id;
    _strcpy(menuItem->text, PLUGIN_MENU_BUFSZ, text);
    _strcpy(menuItem->icon, PLUGIN_MENU_BUFSZ, icon);
    return menuItem;
}

/* Some makros to make the code to create menu items a bit more readable */
#define BEGIN_CREATE_MENUS(x) const size_t sz = x + 1; size_t n = 0; *menuItems = (struct PluginMenuItem**)malloc(sizeof(struct PluginMenuItem*) * sz);
#define CREATE_MENU_ITEM(a, b, c, d) (*menuItems)[n++] = createMenuItem(a, b, c, d);
#define END_CREATE_MENUS (*menuItems)[n++] = NULL; assert(n == sz);


/*
 * Initialize plugin menus.
 * This function is called after ts3plugin_init and ts3plugin_registerPluginID. A pluginID is required for plugin menus to work.
 * Both ts3plugin_registerPluginID and ts3plugin_freeMemory must be implemented to use menus.
 * If plugin menus are not used by a plugin, do not implement this function or return NULL.
 */
void ts3plugin_initMenus(struct PluginMenuItem*** menuItems, char** menuIcon) {
    /*
     * Create the menus
     * There are three types of menu items:
     * - PLUGIN_MENU_TYPE_CLIENT:  Client context menu
     * - PLUGIN_MENU_TYPE_CHANNEL: Channel context menu
     * - PLUGIN_MENU_TYPE_GLOBAL:  "Plugins" menu in menu bar of main window
     *
     * Menu IDs are used to identify the menu item when ts3plugin_onMenuItemEvent is called
     *
     * The menu text is required, max length is 128 characters
     *
     * The icon is optional, max length is 128 characters. When not using icons, just pass an empty string.
     * Icons are loaded from a subdirectory in the TeamSpeak client plugins folder. The subdirectory must be named like the
     * plugin filename, without dll/so/dylib suffix
     * e.g. for "test_plugin.dll", icon "1.png" is loaded from <TeamSpeak 3 Client install dir>\plugins\test_plugin\1.png
     */

    QApplication::instance()->installTranslator(&translator);

    BEGIN_CREATE_MENUS(3);  /* IMPORTANT: Number of menu items must be correct! */
    CREATE_MENU_ITEM(PLUGIN_MENU_TYPE_GLOBAL,  MENU_ID_GLOBAL_1,  (const char*)(QObject::tr("config").toUtf8().constData()),  "");
    CREATE_MENU_ITEM(PLUGIN_MENU_TYPE_GLOBAL,  MENU_ID_GLOBAL_2,  (const char*)(QObject::tr("Enable current server").toUtf8().constData()),  "");
    CREATE_MENU_ITEM(PLUGIN_MENU_TYPE_GLOBAL,  MENU_ID_GLOBAL_3,  (const char*)(QObject::tr("Disable current server").toUtf8().constData()),  "");
    END_CREATE_MENUS;  /* Includes an assert checking if the number of menu items matched */

    /*
     * Specify an optional icon for the plugin. This icon is used for the plugins submenu within context and main menus
     * If unused, set menuIcon to NULL
     */
    menuIcon = NULL;

    /*
     * Menus can be enabled or disabled with: ts3Functions.setPluginMenuEnabled(pluginID, menuID, 0|1);
     * Test it with plugin command: /test enablemenu <menuID> <0|1>
     * Menus are enabled by default. Please note that shown menus will not automatically enable or disable when calling this function to
     * ensure Qt menus are not modified by any thread other the UI thread. The enabled or disable state will change the next time a
     * menu is displayed.
     */
    /* For example, this would disable MENU_ID_GLOBAL_2: */
    /* ts3Functions.setPluginMenuEnabled(pluginID, MENU_ID_GLOBAL_2, 0); */

    /* All memory allocated in this function will be automatically released by the TeamSpeak client later by calling ts3plugin_freeMemory */
}

/************************** TeamSpeak callbacks ***************************/
/*
 * Following functions are optional, feel free to remove unused callbacks.
 * See the clientlib documentation for details on each function.
 */

/* Clientlib */

int ts3plugin_onTextMessageEvent(uint64 serverConnectionHandlerID, anyID targetMode, anyID toID, anyID fromID, const char* fromName, const char* fromUniqueIdentifier, const char* message, int ffIgnored) {
    anyID myID;
    if( targetMode == TextMessageTargetMode::TextMessageTarget_CLIENT && !Globals::textMessageTargetModeClient)
    {
        return 0;
    }

    if( targetMode == TextMessageTargetMode::TextMessageTarget_CHANNEL && !Globals::textMessageTargetModeChannel)
    {
        return 0;
    }
    if( targetMode == TextMessageTargetMode::TextMessageTarget_SERVER && !Globals::textMessageTargetModeServer)
    {
        return 0;
    }

    if(ffIgnored) {
        return 0;
    }

    if(ts3Functions.getClientID(serverConnectionHandlerID, &myID) != ERROR_ok) {
        return 0;
    }

    if ((fromID != myID || Globals::sendToMyself) && hash[serverConnectionHandlerID]) {
        QByteArray messageb = QByteArray::fromStdString(QString(message).toStdString());
        QByteArray fromNameb = QByteArray::fromStdString(QString(fromName).toStdString());
        QTcpSocket socket;
        socket.connectToHost("localhost", 50001);
        socket.waitForConnected();
        char buf[15];
        *((short*)&buf[0])  = 0x0001; //[0-1]  (16Bit) コマンド          （ 0:メッセージ読み上げ）
        *((short*)&buf[2])  =  -1;  //[2-3]  (16Bit) 速度              （-1:棒読みちゃん画面上の設定）
        *((short*)&buf[4])  =  -1;   //[4-5]  (16Bit) 音程              （-1:棒読みちゃん画面上の設定）
        *((short*)&buf[6])  =  -1; //[6-7]  (16Bit) 音量              （-1:棒読みちゃん画面上の設定）
        *((short*)&buf[8])  = 0;  //[8-9]  (16Bit) 声質              （ 0:棒読みちゃん画面上の設定、1:女性1、2:女性2、3:男性1、4:男性2、5:中性、6:ロボット、7:機械1、8:機械2、10001～:SAPI5）
        *((char* )&buf[10]) = 0;      //[10]   ( 8Bit) 文字列の文字コード（ 0:UTF-8, 1:Unicode, 2:Shift-JIS）
        *((long* )&buf[11]) = messageb.length() + 1 + fromNameb.length();    //[11-14](32Bit) 文字列の長さ
        socket.write(buf,15);
        socket.write(fromNameb);
        socket.write(" ");
        socket.write(messageb);
        socket.waitForBytesWritten();
        socket.disconnectFromHost();
    }
    return 0;
}
/*
 * Called when a plugin menu item (see ts3plugin_initMenus) is triggered. Optional function, when not using plugin menus, do not implement this.
 *
 * Parameters:
 * - serverConnectionHandlerID: ID of the current server tab
 * - type: Type of the menu (PLUGIN_MENU_TYPE_CHANNEL, PLUGIN_MENU_TYPE_CLIENT or PLUGIN_MENU_TYPE_GLOBAL)
 * - menuItemID: Id used when creating the menu item
 * - selectedItemID: Channel or Client ID in the case of PLUGIN_MENU_TYPE_CHANNEL and PLUGIN_MENU_TYPE_CLIENT. 0 for PLUGIN_MENU_TYPE_GLOBAL.
 */
void ts3plugin_onMenuItemEvent(uint64 serverConnectionHandlerID, enum PluginMenuType type, int menuItemID, uint64 selectedItemID) {
    QApplication::instance()->installTranslator(&translator);
    BouyomiConfigDialog qConfigDialog;

    switch(type) {
        case PLUGIN_MENU_TYPE_GLOBAL:
            /* Global menu item was triggered. selectedItemID is unused and set to zero. */
            switch(menuItemID) {
                case MENU_ID_GLOBAL_1:
                    /* Menu global 1 was triggered */

                    qConfigDialog.exec();
                    break;
                case MENU_ID_GLOBAL_2:
                    ts3Functions.setPluginMenuEnabled(pluginID, MENU_ID_GLOBAL_2, 0);
                    ts3Functions.setPluginMenuEnabled(pluginID, MENU_ID_GLOBAL_3, 1);
                    hash[serverConnectionHandlerID] = 1;
                    break;
                case MENU_ID_GLOBAL_3:
                    ts3Functions.setPluginMenuEnabled(pluginID, MENU_ID_GLOBAL_2, 1);
                    ts3Functions.setPluginMenuEnabled(pluginID, MENU_ID_GLOBAL_3, 0);
                    hash[serverConnectionHandlerID] = 0;
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
}
