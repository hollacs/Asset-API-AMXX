#include <amxmodx>
#include <amxmisc>
#include <json>

#define VERSION "0.2"

new g_fwParseJson;

public plugin_precache()
{
	g_fwParseJson = CreateMultiForward("asset_OnParseJson", ET_IGNORE, FP_CELL, FP_STRING, FP_STRING);
}

public plugin_init()
{
	register_plugin("Asset API", VERSION, "holla");
}

public plugin_natives()
{
	register_library("asset_api");

	register_native("asset_loadJson", "native_loadJson");
	register_native("asset_passJson", "native_passJson");
}

public native_loadJson()
{
	enum { argName=1, argPath, argRelativePath };

	static filePath[128];

	if (get_param(argRelativePath))
	{
		static path[64];
		get_configsdir(filePath, charsmax(filePath));
		get_string(argPath, path, charsmax(path));
		format(filePath, charsmax(filePath), "%s/%s", filePath, path);
	}
	else
	{
		get_string(argPath, filePath, charsmax(filePath));
	}

	static ret;
	if (!file_exists(filePath))
	{
		ExecuteForward(g_fwParseJson, ret, Invalid_JSON, name, filePath);
		log_amx("file '%s' not exist", filePath);
		return 0;
	}

	new JSON:json = json_parse(filePath, true, true);
	if (json == Invalid_JSON)
	{
		ExecuteForward(g_fwParseJson, ret, Invalid_JSON, name, filePath);
		log_amx("invalid json file '%s'", filePath);
		return 0;
	}

	static name[64];
	get_string(argName, name, charsmax(name));

	ExecuteForward(g_fwParseJson, ret, json, name, filePath);

	json_free(json);
	return 1;
}

public native_passJson()
{
	enum { argJson=1, argName, argPath };

	new JSON:json = JSON:get_param(argJson);

	static name[64], path[128];
	get_string(argName, name, charsmax(name));
	get_string(argPath, path, charsmax(path));

	static ret;
	ExecuteForward(g_fwParseJson, ret, json, name, path);
}