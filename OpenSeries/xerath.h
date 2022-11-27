#pragma once
#include "../plugin_sdk/plugin_sdk.hpp"

namespace xerath
{
	inline bool isMoving(const game_object_script& target);
	inline float timeBeforeWHits(const game_object_script& target);
	inline float timeBeforeWHitsLocation(vector& position);
	inline bool willGetHitByE(const game_object_script& target);
	inline bool willGetHitByR(const game_object_script& target);
	inline float getPing();
	inline float getGodBuffTime(const game_object_script& target);
	inline float getNoKillBuffTime(const game_object_script& target);
	inline float getStasisTime(const game_object_script& target);
	inline float getTotalHP(const game_object_script& target);
	inline bool couldDamageLater(const game_object_script& target, const float time, float damage);
	inline float getTimeToHit(prediction_input& input, prediction_output& predInfo, const bool takePing);
	inline int alliesAroundTarget(const game_object_script& target, float range);
	inline hit_chance getPredIntFromSettings(const int hitchance);
	inline float charged_percentage(const float spell_charge_duration);
	inline float true_charged_percentage(const float spell_charge_duration);
	inline float charged_range(const float max_range, const float min_range, const float duration);
	inline float true_charged_range(const float max_range, const float min_range, const float duration);
	inline bool castQShort(const game_object_script& target, std::string mode);
	inline bool castQCharge(const game_object_script& target, std::string mode);
	inline bool castQLong(const game_object_script& target, std::string mode);
	inline bool castW(const game_object_script& target, std::string mode, bool wCenter);
	inline bool castE(const game_object_script& target, std::string mode);
	inline bool castR(const game_object_script& target, std::string mode);
	inline prediction_output getQShortPred(const game_object_script& target);
	inline prediction_output getWPred(const game_object_script& target);
	inline prediction_output getEPred(const game_object_script& target);
	inline prediction_output getRPred(const game_object_script& target);
	inline prediction_output getQDummyPred(const game_object_script& target);
	inline prediction_output getQ2Pred(const game_object_script& target);
	inline float getExtraDamage(const game_object_script& target, const int shots, const float predictedHealth, const float damageDealt, const bool isCC, const bool firstShot, const bool isTargeted);
	inline float getQDamage(const game_object_script& target);
	inline float getQDamageAlternative(const game_object_script& target, const int shots, const float predictedHealth, const int firstShot);
	inline float getWDamage(const game_object_script& target);
	inline float getW2Damage(const game_object_script& target);
	inline float getW2DamageAlternative(const game_object_script& target, const int shots, const float predictedHealth, const int firstShot);
	inline float getEDamage(const game_object_script& target);
	inline float getEDamageAlternative(const game_object_script& target, const int shots, const float predictedHealth, const int firstShot);
	inline float getRDamage(const game_object_script& target, const int shots, const float predictedHealth, const bool firstShot);
	inline void draw_dmg_rl(const game_object_script& target, const float damage, unsigned long color);
	inline void draw_dmg_lr(const game_object_script& target, const float damage, unsigned long color);
	inline bool isYuumiAttached(const game_object_script& target);
	inline bool isRecalling(const game_object_script& target);
	inline bool isValidRecalling(const game_object_script& target, float range, const vector& from);
	inline bool customIsValid(const game_object_script& target, float range, const vector& from, bool invul);
	inline bool limitedTick(int msTime);
	inline void calcs();
	inline bool debuffCantCast();
	inline bool isCastingSpell();
	inline bool canCastSpells();
	inline void targetSelectorSort();
	inline void manualHandling();
	inline void combo();
	inline void harass();
	inline void particleHandling();
	inline void automatic();
	inline void createMenu();
	inline void on_update();
	inline void on_draw();
	inline void on_create(const game_object_script obj);
	inline void on_delete(const game_object_script obj);
	inline void on_buff(game_object_script& sender, buff_instance_script& buff, const bool gain);
	inline void on_buff_gain(game_object_script sender, buff_instance_script buff);
	inline void on_buff_lose(game_object_script sender, buff_instance_script buff);
	inline void on_cast_spell(spellslot spellSlot, game_object_script target, vector& pos, vector& pos2, bool isCharge, bool* process);
	inline void on_process_spell_cast(game_object_script sender, spell_instance_script spell);
	void load();
	void unload();
};
