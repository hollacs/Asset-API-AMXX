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

public plugin_precache()
{
	// 載入 JSON
	if (!asset_loadJson("test", "test.json"))
		set_fail_state("failed to load json file"); // 若載入失敗則停止插件
}

public asset_OnParseJson(JSON:json, const name[])
{
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
}

public plugin_init()
{
	register_plugin("Asset API Test", "0.1", "holla");

	register_clcmd("test_spk", 	"CmdTestSpk");
	register_clcmd("test_emit", "CmdTestEmit");

	RegisterHam(Ham_Spawn, 			"player", 		"OnPlayerSpawn_Post", 1);
	RegisterHam(Ham_Item_Deploy, 	"weapon_knife", "OnKnifeDeploy_Post", 1);
}

public CmdTestSpk()
{
	ARRAY_RANDOM_STR(g_soundSpk, sound[64]) // 獲得隨機音效
	client_cmd(0, "spk %s", sound); // 用 spk 方式播放
}

public CmdTestEmit(id)
{
	// 用 emit_sound 播放音效在玩家身上
	emit_sound(id, CHAN_AUTO, g_soundEmit, VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
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