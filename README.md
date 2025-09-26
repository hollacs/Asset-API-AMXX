# Asset API AMXX

**Requirement**: AMXX 1.9 or higher

This is a simplified, non-object-oriented version of the Asset Manager I previously released. The earlier design had some flaws and was overly complex, so I reworked it into this new API.  
In short, this is a JSON-based API that allows plugins to easily read the locations of game resource files, avoiding hardcoding paths in plugins.

- `asset_api.inc` should be placed in `scripting/include`
- `asset_api.sma` needs to be added to `plugins.ini`
- `asset_api_test.sma` is a test plugin
- `configs/test.json` is the JSON file for testing

Use it by including:  
```sourcepawn
#include <asset_api>
```

**Usage Example**:
```sourcepawn
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
    // Load JSON
    if (!asset_loadJson("test", "test.json"))
        set_fail_state("failed to load json file"); // Stop plugin if JSON loading fails
}

// Handle JSON loading
public asset_OnHandleJson(JSON:json, const name[])
{
    if (!equal(name, "test")) return;

    // Load spk sound
    g_soundSpk = asset_toArray(Asset_Generic, json, "spk", 64, 
        .defaultFile="sound/events/enemy_died.wav");

    // Load emit sound
    asset_toString(Asset_Sound, json, "emit", g_soundEmit, charsmax(g_soundEmit), 
        .defaultFile="player/headshot1.wav");

    // Load player model
    asset_toString(Asset_PlayerModel, json, "playermodel", g_playerModel, charsmax(g_playerModel), 
        .defaultFile="terror");

    // Load knife model
    asset_toString(Asset_Model, json, "v_knife", g_knifeModel, charsmax(g_knifeModel), 
        .defaultFile="models/v_knife.mdl");

    // Since the JSON object is passed, you can still perform regular JSON operations here
}

public plugin_init()
{
    register_plugin("Asset API Test", "0.1", "holla");

    register_clcmd("test_spk", "CmdTestSpk");
    register_clcmd("test_emit", "CmdTestEmit");

    RegisterHam(Ham_Spawn, "player", "OnPlayerSpawn_Post", 1);
    RegisterHam(Ham_Item_Deploy, "weapon_knife", "OnKnifeDeploy_Post", 1);
}

public CmdTestSpk()
{
    ARRAY_RANDOM_STR(g_soundSpk, sound[64]) // Get random sound
    client_cmd(0, "spk %s", sound); // Play sound using spk
}

public CmdTestEmit(id)
{
    // Play sound on player using emit_sound
    emit_sound(id, CHAN_AUTO, g_soundEmit, VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
}

public OnPlayerSpawn_Post(id)
{
    if (!is_user_alive(id)) return;

    cs_set_user_model(id, g_playerModel); // Change player model
}

public OnKnifeDeploy_Post(ent)
{
    if (!is_valid_ent(ent)) return;

    new id = get_ent_data_entity(ent, "CBasePlayerItem", "m_pPlayer");
    if (id)
        entity_set_string(id, EV_SZ_viewmodel, g_knifeModel); // Change knife model
}
```

**Test JSON Configuration File**:
```json
{
    "spk": ["sound/hostage/hos1.wav", "sound/hostage/hos2.wav", "sound/hostage/hos3.wav"],
    "emit": "scientist/scream1.wav",
    "playermodel": "vip",
    "v_knife": "models/v_knife_r.mdl"
}
```

---

**Using with SVC_TEMPENTITY SPR**:
If you want to use it for SPR in SVC_TEMPENTITY, you can write it like this:
```sourcepawn
new g_spr;

public asset_OnHandleJson(JSON:json, const name[])
{
    if (!equal(name, "test")) return;
    g_spr = asset_toString(Asset_Model, json, "spr");
}
```

```json
    "spr": "sprites/zbeam4.spr"
```

---

**Handling Nested JSON Objects**:  
If your data is stored in a nested JSON object, like this:
```json
{
    "object": {
        "test_model": "models/head.mdl"
    }
}
```

Since dot notation is supported, you can write:
```sourcepawn
asset_toString(Asset_Model, json, "object.test_model", g_testModel, charsmax(g_testModel));
```
