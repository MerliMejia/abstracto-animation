# abstracto-animation

Header-only skeletal animation and playback module for the Abstracto ecosystem. Its job is to represent skeleton data, evaluate animation clips, and keep per-model
playback state.

## What it does

- Stores skeleton hierarchies, skins, joints, and animation clips
- Stores local and world runtime pose data
- Samples translation, rotation, and scale tracks into a pose
- Keeps playback state such as selected clip, current time, speed, play/pause,
  and looping
- Resets poses back to bind pose when needed

## Main types

- `SkeletonAssetData`
  Full imported skeleton data: nodes, scene roots, skins, and animations.
- `SkeletonNode`
  A skeleton node with parent/child links and a local bind transform.
- `SkinData`
  Joint list plus inverse bind matrices for a skin.
- `AnimationClipData`
  A named clip made of node animation tracks.
- `NodeAnimationTrack`
  Per-node animation data for translation, rotation, or scale.
- `SkeletonPose`
  Runtime pose with local transforms and computed world transforms.
- `AnimationPlaybackState`
  Selected animation index, current time, speed, play/pause, and loop flags.
- `ModelAnimationState`
  Small state wrapper that owns a `SkeletonPose` plus `AnimationPlaybackState`
  and exposes the normal playback operations.

## Current features

- Bind-pose initialization from imported skeleton data
- Runtime pose storage per node
- Hierarchical world-transform recomputation
- Translation and scale sampling with linear interpolation and step mode
- Rotation sampling with quaternion slerp and step mode
- Clip selection
- Play, pause, reset, and loop support
- Positive or negative playback speed support through time accumulation

## Current limitations

- No animation blending between clips
- No state machine or transition system
- No root motion extraction
- No events/notifies
- No retargeting
- `CubicSpline` exists in the data model, but sampling currently treats all
  non-`Step` tracks as linear interpolation / slerp

## How it works

The normal flow is:

1. Fill a `SkeletonAssetData` from an importer.
2. Create a `ModelAnimationState`.
3. Call `reset(&skeleton)` to create the initial pose from bind pose.
4. Choose a clip with `selectSourceAnimation(...)`.
5. Start playback with `playSelectedAnimation(...)`.
6. Call `updatePlayback(&skeleton, deltaSeconds)` every frame.
7. Read the evaluated pose through `currentPose(...)`.

When a clip is sampled, the module:

1. Resets the pose to bind pose
2. Applies animated translation, rotation, and scale channels to the affected
   nodes
3. Recomputes world transforms for the full hierarchy

That means nodes without animation tracks stay at bind-pose values.

## Minimal usage

```cpp
#include "animation/ModelAnimationState.h"

SkeletonAssetData skeleton;
// Fill skeleton.nodes, skeleton.skins, and skeleton.animations from your importer.

ModelAnimationState animationState;
animationState.reset(&skeleton);

animationState.selectSourceAnimation(&skeleton, 0);
animationState.playSelectedAnimation(&skeleton);

const float deltaSeconds = 1.0f / 60.0f;
animationState.updatePlayback(&skeleton, deltaSeconds);

const SkeletonPose *pose = animationState.currentPose(&skeleton);
if (pose != nullptr && !pose->empty()) {
  const glm::mat4 rootWorld = pose->worldTransform(0);
  (void)rootWorld;
}
```

## Build

```bash
cmake -S . -B build
cmake --build build -j4
./build/AbstractoAnimationExample
```

## Where to see it in use

- Minimal example target:
  [`apps/animation_example/main.cpp`](/Users/merlimejia/Desktop/Guandul/Game Dev/abstracto-dev/abstracto-animation/apps/animation_example/main.cpp)
- Build output:
  `build/AbstractoAnimationExample`

## Example target

`AbstractoAnimationExample` demonstrates:

- creating a small character skeleton
- defining a clip with translation and rotation tracks
- owning `ModelAnimationState` as part of an engine-side character object
- ticking animation playback once per frame
- querying a runtime socket/bone transform after animation evaluation

That is the intended usage pattern for this module: keep animation state close
to your runtime entity or component, update it during your engine tick, and use
the resulting `SkeletonPose` for skinning, attachments, or gameplay queries.
