#include "animator_component.h"

void AnimatorComponent::LoadAnimator(std::istream& s) { 
    KeyframeAnimationLoader loader;
    auto animator = loader.LoadAnimation(s);
    if (animator) {
        AddAnimator(animator);
        Play(animator->clip_name);
    } 
}

