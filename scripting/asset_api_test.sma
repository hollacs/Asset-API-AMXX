#include <amxmodx>
#include <cstrike>
#include <engine>
#include <fakemeta>
#include <hamsandwich>
#include <asset_api>

new Array:g_soundSpk;
new g_soundEmit[64];
new g_playerModel[32];
new g_knifeModel[64];
new g_sprDisk;

public plugin_precache()
{
	// 載入 JSON
	// 若載入失敗它也會至少以 Invalid_JSON 來呼叫一次 asset_OnParseJson 來預載預設的資源檔案
	asset_loadJson("test", "test.json")
}

public asset_OnParseJson(JSON:json, const name[])
{
	// 檢查識別的名字
	if (!equal(name, "test")) return;

	// 載入 spk 音效
	g_soundSpk = asset_toArray(Asset_Generic, json, "spk", 64, 
		.defaultFile="sound/events/enemy_died.wav");

	// 載入 emit 音效
	asset_toString(Asset_Sound, json, "emit", g_soundEmit, charsmax(g_soundEmit), 
		.defaultFile="player/headshot1.wav");

	// 載入玩家模組
	asset_toString(Asset_PlayerModel, json, "playermodel", g_playerModel, charsmax(g_playerModel), 
		.defaultFile="terror");

	// 載入小刀模組
	asset_toString(Asset_Model, json, "v_knife", g_knifeModel, charsmax(g_knifeModel), 
		.defaultFile="models/v_knife.mdl");

	// 載入 SPR
	g_sprDisk = asset_toString(Asset_Model, json, "spr_disk",
		.defaultFile="sprites/zbeam1.spr");
}

public plugin_init()
{
	register_plugin("Asset API Test", "0.1", "holla");

	register_clcmd("test_spk", 	"CmdTestSpk");
	register_clcmd("test_emit", "CmdTestEmit");
	register_clcmd("test_spr",  "CmdTestSpr");

	RegisterHam(Ham_Spawn, 			"player", 		"OnPlayerSpawn_Post", 1);
	RegisterHam(Ham_Item_Deploy, 	"weapon_knife", "OnKnifeDeploy_Post", 1);
}

public CmdTestSpk()
{
	ARRAY_RANDOM_STR(g_soundSpk, sound[64]) // 獲得隨機音效 (這是 INC 裡面的 macro)
	client_cmd(0, "spk %s", sound); // 用 spk 方式播放
}

public CmdTestEmit(id)
{
	// 用 emit_sound 播放音效在玩家身上
	emit_sound(id, CHAN_AUTO, g_soundEmit, VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
}

public CmdTestSpr(id)
{
	new origin[3];
	get_user_origin(id, origin, 3);
	origin[2] += 1;

	message_begin(MSG_BROADCAST ,SVC_TEMPENTITY) //message begin
	write_byte(TE_BEAMDISK)
	write_coord(origin[0]) // center position
	write_coord(origin[1])
	write_coord(origin[2])
	write_coord(origin[0]) // axis and radius
	write_coord(origin[1])
	write_coord(origin[2] + 250)
	write_short(g_sprDisk) // sprite index
	write_byte(0) // starting frame
	write_byte(0) // frame rate in 0.1's
	write_byte(10) // life in 0.1's
	write_byte(10) // line width in 0.1's
	write_byte(0) // noise amplitude in 0.01's
	write_byte(0) //colour
	write_byte(100)
	write_byte(200)
	write_byte(255) // brightness
	write_byte(0) // scroll speed in 0.1's
	message_end()
}

public OnPlayerSpawn_Post(id)
{
	if (!is_user_alive(id)) return;

	cs_set_user_model(id, g_playerModel); // 改變玩家模組
}

public OnKnifeDeploy_Post(ent)
{
	if (!is_valid_ent(ent)) return;

	new id = get_ent_data_entity(ent, "CBasePlayerItem", "m_pPlayer");
	if (id)
		entity_set_string(id, EV_SZ_viewmodel, g_knifeModel); // 改變小刀模組
}