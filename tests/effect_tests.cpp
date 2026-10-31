#include <cassert>

#include "audio/AudioManager.h"
#include "effects/EffectManager.h"

int main() {
    EffectManager effects;
    effects.emit({EffectEventKind::GoldImpact, 300.0f, 200.0f});
    effects.emit({EffectEventKind::CurseImpact, 500.0f, 300.0f});
    effects.emit({EffectEventKind::CoinPickup, 600.0f, 400.0f});
    assert(effects.particles().size() == 34);
    const float initialLife = effects.particles().front().life;
    effects.update(0.1f);
    assert(effects.particles().front().life < initialLife);
    for (int i = 0; i < 20; ++i) effects.update(0.1f);
    assert(effects.particles().empty());
    effects.setOutcome(EffectOutcome::Victory);
    assert(effects.outcome() == EffectOutcome::Victory);
    assert(effects.particles().size() == 32);
    effects.update(0.3f);
    assert(effects.particles().size() > 32);
    effects.reset();
    assert(effects.outcome() == EffectOutcome::None);
    assert(effects.particles().empty());

    AudioSettings audio;
    assert(audio.volume == 0.6f && !audio.muted);
    audio.adjust(1.0f);
    assert(audio.volume == 1.0f);
    audio.adjust(-2.0f);
    assert(audio.volume == 0.0f);
    audio.toggleMute();
    assert(audio.muted);
    audio.toggleMute();
    assert(!audio.muted);
}
