#include "../plugin_sdk/plugin_sdk.hpp"
#include "brand.h"
#include "xerath.h"

PLUGIN_NAME("OpenSeries");

PLUGIN_TYPE(plugin_type::champion);
SUPPORTED_CHAMPIONS( champion_id::Brand, champion_id::Xerath );

PLUGIN_API bool on_sdk_load( plugin_sdk_core* plugin_sdk_good )
{
    DECLARE_GLOBALS( plugin_sdk_good );

    switch (myhero->get_champion())
    {
    case champion_id::Brand:
        // Load Brand script
        //
        brand::load();
        break;
    case champion_id::Xerath:
        // Load Xerath script
        //
        xerath::load();
        break;
    default:
        // We don't support this champ, print message and return false (core will not load this plugin and on_sdk_unload will be never called)
        //
        console->print("Champion %s is not supported!", myhero->get_model_cstr());
        return false;
    }
    return true;
}

PLUGIN_API void on_sdk_unload( )
{
    brand::unload();
    xerath::unload();

}