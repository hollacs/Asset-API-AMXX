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

// 遊戲資源類型
enum AssetType
{
	Asset_Model, 		// 模組 (例如: .mdl, .spr)
	Asset_Sound, 		// 音效 (例如: .wav)
	Asset_Generic, 		// 通用 (例如: .wav, .mp3, .txt, ...)
	Asset_PlayerModel 	// 玩家模型
};

/*
 * 會在使用 asset_loadJson 或 asset_passJson 時被呼叫
 *
 * @param json 			JSON 物件
 * @param name 			識別名字
 * @param filePath 		檔案位置
 * 
 * @noreturn
 */
forward asset_OnParseJson(JSON:json, const name[], const filePath[]);

/*
 * 載入 JSON
 *
 * @param name 			用以識別的名字
 * @param filePath 		要讀取的檔案位置
 * @param relativePath 	如果是 true 使相對的位置 (amxmodx/configs) 否則將使用絕對位置
 * 
 * @return				0 代表載入失敗, 1 代表載入成功
 */
native asset_loadJson(const name[], const filePath[], bool:relativePath=true);

/*
 * 傳遞 JSON 物件到 asset_OnParseJson (forward 會被呼叫一次)
 *
 * @note 當你不想用 asset_loadJson 來幫你載入 json 就用這個
 *
 * @param json 			JSON 物件
 * @param name 			用以識別的名字
 * @param filePath 		檔案位置 (可留空)
 * 
 * @noreturn
 */
native asset_passJson(JSON:json, const name[], const filePath[]="");

/*
 * 讀取 JSON 物件的遊戲資源到字串 (會自動預載)
 *
 * @param type 			遊戲資源的類型
 * @param json 			JSON 物件
 * @param key 			要被讀取的鍵值
 * @param string 		輸出到的字串陣列
 * @param length		字串長度
 * @param defaultFile 	當 JSON 物件沒有讀取到東西就使用這個檔案 (可留空)
 * @param dotNotation 	JSON 使用 dot notation
 * @param useValveFs 	使用 valve 的檔案系統 (意思是包含 valve/ 裡面的檔案)
 * 
 * @return 				回傳 precache 的 id (0 為失敗)
 */
stock asset_toString(
	AssetType:type, JSON:json, const key[], string[]="", length=0, 
	const defaultFile[]="", bool:dotNotation=true, bool:useValveFs=true)
{
	if (json == Invalid_JSON)
		return 0;

	if (!json_object_has_value(json, key, JSONString, dotNotation))
	{
		if (defaultFile[0])
			copy(string, length, defaultFile);
		
		return asset_precache(type, defaultFile, useValveFs);
	}
	
	json_object_get_string(json, key, string, length, dotNotation);
	return asset_precache(type, string, useValveFs);
}

/*
 * 讀取 JSON 物件的遊戲資源到動態陣列 (會自動預載)
 *
 * @param type 			遊戲資源的類型
 * @param json 			JSON 物件
 * @param key 			要被讀取的鍵值
 * @param stringLength	字串長度 (ArrayCreate)
 * @param defaultFile 	當 JSON 物件沒有讀取到東西就使用這個檔案 (可留空)
 * @param dotNotation 	JSON 使用 dot notation
 * @param useValveFs 	使用 valve 的檔案系統 (意思是包含 valve/ 裡面的檔案)
 * 
 * @return 				回傳 Array: 的 handle
 */
stock Array:asset_toArray(
	AssetType:type, JSON:json, const key[], stringLength,
	const defaultFile[]="", bool:dotNotation=true, bool:useValveFs=true)
{
	if (json == Invalid_JSON)
		return Invalid_Array;

	new Array:aArray = ArrayCreate(stringLength);

	if (!json_object_has_value(json, key, JSONArray, dotNotation))
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

		if (asset_precache(type, file, useValveFs))
			ArrayPushString(aArray, file);
	}
	json_free(jArray);

	if (ArraySize(aArray) < 1)
		ArrayDestroy(aArray);
	
	return aArray;
}

/*
 * 預載遊戲資源
 *
 * @param type 			遊戲資源的類型
 * @param file			檔案位置
 * @param useValveFs 	使用 valve 的檔案系統 (意思是包含 valve/ 裡面的檔案)
 * 
 * @return 				回傳 precache 的 id
 */
stock asset_precache(AssetType:type, const file[], bool:useValveFs=true)
{
	if (!file[0])
		return 0;

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
		return 0;
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

	return 0;
}
