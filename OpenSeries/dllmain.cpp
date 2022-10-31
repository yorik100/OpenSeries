#include "../plugin_sdk/plugin_sdk.hpp"
#include "brand.h"

PLUGIN_NAME("OpenBrand");

SUPPORTED_CHAMPIONS(champion_id::Brand);

PLUGIN_API bool on_sdk_load( plugin_sdk_core* plugin_sdk_good )
{
    DECLARE_GLOBALS( plugin_sdk_good );

    brand::load();

    return true;
}

PLUGIN_API void on_sdk_unload( )
{

}