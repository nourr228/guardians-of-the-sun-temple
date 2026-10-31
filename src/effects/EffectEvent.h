#pragma once

enum class EffectEventKind {
    GoldImpact, CurseImpact, CoinPickup, EnemyDefeat,
    ArrowLaunch, CurseLaunch
};

struct EffectEvent {
    EffectEventKind kind;
    float x;
    float y;
};
