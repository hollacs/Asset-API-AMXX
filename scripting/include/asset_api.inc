#if defined _asset_api_included
	#endinput
#endif
#define _asset_api_included

#include <json>

// useless macros
#define ARRAY_RANDOM_STR(%0,%1[%2]) static %1[%2]; ArrayGetString(%0,random(ArraySize(%0)),%1,%2-1)
//#define SPK_SOUND(%0,%1) client_cmd(%0, "spk %s", %1)
//#define SPK_RANDOM(%0,%1) if (%1) { ARRAY_RANDOM_STR(%1,sound[64]); SPK_SOUND(%0,sound); }
//#define EMIT_SOUND_RANDOM(%0,%1,%2,%3,%4,%5,%6) if (%2) { ARRAY_RANDOM_STR(%2,sound[64]); emit_sound(%0,%1,sound,%3,%4,%5,%6); }

// Game Resource Type
enum AssetType
{
	Asset_Model, 		// Model (exmaple: .mdl, .spr)
	Asset_Sound, 		// Sound (exmaple: .wav)
	Asset_Generic, 		// Generic (exmaple: .wav, .mp3, .txt, ...)
	Asset_PlayerModel 	// Player Model
};

/*
 * called when using asset_loadJson or asset_passJson
 *
 * @param json 			JSON object
 * @param name 			name to identify
 * @param filePath 		file path
 * 
 * @noreturn
 */
forward asset_OnHandleJson(JSON:json, const name[], const filePath[]);

/*
 * Load JSON
 *
 * @param name 			name to identify
 * @param filePath 		file path
 * @param relativePath 	if true use relativepath (amxmodx/configs) oterwise use absolute path
 * 
 * @return				0 is fail, 1 is success
 */
native asset_loadJson(const name[], const filePath[], bool:relativePath=true);

/*
 * Pass JSON object to asset_OnHandleJson (forward will be called once)
 *
 * @note use this if you your json object isn't a file or you have already loaded before.
 *
 * @param json 			JSON object
 * @param name 			name to identify
 * @param filePath 		file path (can be empty)
 * 
 * @noreturn
 */
native asset_passJson(JSON:json, const name[], const filePath[]="");

/*
 * Read JSON object to a string (precache automatcally)
 *
 * @param type 			resource type
 * @param json 			JSON object
 * @param key 			key in json
 * @param string 		string output
 * @param length		string length
 * @param defaultFile 	if JSON read nothings, use this file (can be empty)
 * @param dotNotation 	JSON uses dot notation
 * @param useValveFs 	use valve file system (means includes file in the valve/ folder)
 * 
 * @return 				precache id (-1 = failure)
 */
stock asset_toString(
	AssetType:type, JSON:json, const key[], string[]="", length=0, 
	const defaultFile[]="", bool:dotNotation=true, bool:useValveFs=true)
{
	if (json == Invalid_JSON || !json_object_has_value(json, key, JSONString, dotNotation))
	{
		if (defaultFile[0])
			copy(string, length, defaultFile);
		
		return asset_precache(type, defaultFile, useValveFs);
	}

	if (length < 1)
	{
		static str[64];
		json_object_get_string(json, key, str, charsmax(str), dotNotation);
		return asset_precache(type, str, useValveFs);
	}
	
	json_object_get_string(json, key, string, length, dotNotation);
	return asset_precache(type, string, useValveFs);
}

/*
 * Read JSON to a cellarray (precache automatcally)
 *
 * @param type 			resource type
 * @param json 			JSON object
 * @param key 			key in json
 * @param stringLength	string length (ArrayCreate)
 * @param defaultFile 	if JSON reads nothing, use this file (can be empty)
 * @param dotNotation 	JSON uses dot notation
 * @param useValveFs 	use valve file system (means includes file in the valve/ folder)
 * 
 * @return 			 	Array: handle
 */
stock Array:asset_toArray(
	AssetType:type, JSON:json, const key[], stringLength,
	const defaultFile[]="", bool:dotNotation=true, bool:useValveFs=true)
{
	new Array:aArray = ArrayCreate(stringLength);

	if (json == Invalid_JSON || !json_object_has_value(json, key, JSONArray, dotNotation))
	{
		if (defaultFile[0])
		{
			ArrayPushString(aArray, defaultFile);
			asset_precache(type, defaultFile, useValveFs);
		}
		else
			ArrayDestroy(aArray);
		
		return aArray;
	}
	
	static file[96];

	new JSON:jArray = json_object_get_value(json, key, dotNotation);
	new count = json_array_get_count(jArray);

	for (new i = 0; i < count; i++)
	{
		json_array_get_string(jArray, i, file, charsmax(file));

		if (asset_precache(type, file, useValveFs) != -1)
			ArrayPushString(aArray, file);
	}
	json_free(jArray);

	if (ArraySize(aArray) < 1)
		ArrayDestroy(aArray);
	
	return aArray;
}

/*
 * precache game resource
 *
 * @param type 			resource type
 * @param file			file path
 * @param useValveFs 	use valve file system (means includes file in the valve/ folder)
 * 
 * @return 				precache id
 */
stock asset_precache(AssetType:type, const file[], bool:useValveFs=true)
{
	if (!file[0])
		return -1;

	static path[96];
	if (type == Asset_PlayerModel)
		formatex(path, charsmax(path), "models/player/%s/%s.mdl", file, file);
	else
		copy(path, charsmax(path), file);

	new bool:fileExists;
	if (type == Asset_Sound)
		fileExists = bool:file_exists(fmt("sound/%s", path), useValveFs);
	else
		fileExists = bool:file_exists(path, useValveFs);

	if (!fileExists)
	{
		static const typeNames[AssetType][] = {"model", "sound", "generic", "playermodel"};
		log_amx("precache(%s): file '%s' not exist", typeNames[type], path);
		return -1;
	}

	switch (type)
	{
		case Asset_Model, Asset_PlayerModel:
			return precache_model(path);
		case Asset_Sound:
			return precache_sound(path);
		case Asset_Generic:
			return precache_generic(path);
	}

	return -1;
}
