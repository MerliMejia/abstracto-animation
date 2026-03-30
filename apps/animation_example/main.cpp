#include "ModelAnimationState.h"
#include <glm/gtc/quaternion.hpp>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <vector>

static NodeAnimationTrack makeTranslationTrack(
    int nodeIndex, std::vector<float> timesSeconds,
    std::vector<glm::vec3> values) {
  return NodeAnimationTrack{
      .targetNodeIndex = nodeIndex,
      .targetPath = AnimationTargetPath::Translation,
      .interpolation = AnimationInterpolation::Linear,
      .timesSeconds = std::move(timesSeconds),
      .vec3Values = std::move(values),
  };
}

static NodeAnimationTrack makeRotationTrack(
    int nodeIndex, std::vector<float> timesSeconds,
    std::vector<glm::quat> values) {
  return NodeAnimationTrack{
      .targetNodeIndex = nodeIndex,
      .targetPath = AnimationTargetPath::Rotation,
      .interpolation = AnimationInterpolation::Linear,
      .timesSeconds = std::move(timesSeconds),
      .quatValues = std::move(values),
  };
}

static glm::vec3 translationFromMatrix(const glm::mat4 &matrix) {
  return glm::vec3(matrix[3]);
}

static glm::vec3 forwardFromMatrix(const glm::mat4 &matrix) {
  return glm::normalize(glm::vec3(matrix[0]));
}

static SkeletonAssetData buildCharacterSkeleton() {
  SkeletonAssetData skeleton;

  skeleton.nodes.push_back(SkeletonNode{
      .name = "hips",
      .parentIndex = -1,
      .childIndices = {1},
      .localBindTransform = NodeTransform{
          .translation = glm::vec3(0.0f, 0.0f, 0.0f),
          .rotation = glm::identity<glm::quat>(),
          .scale = glm::vec3(1.0f),
      },
  });

  skeleton.nodes.push_back(SkeletonNode{
      .name = "weapon_socket",
      .parentIndex = 0,
      .childIndices = {},
      .localBindTransform = NodeTransform{
          .translation = glm::vec3(0.65f, 0.35f, 0.0f),
          .rotation = glm::identity<glm::quat>(),
          .scale = glm::vec3(1.0f),
      },
  });

  skeleton.sceneRootNodeIndices.push_back(0);

  skeleton.animations.push_back(AnimationClipData{
      .name = "RunAndAim",
      .tracks =
          {
              makeTranslationTrack(
                  0, {0.0f, 0.5f, 1.0f},
                  {
                      glm::vec3(0.0f, 0.0f, 0.0f),
                      glm::vec3(0.8f, 0.0f, 0.0f),
                      glm::vec3(1.6f, 0.0f, 0.0f),
                  }),
              makeRotationTrack(
                  1, {0.0f, 0.5f, 1.0f},
                  {
                      glm::angleAxis(glm::radians(-20.0f),
                                     glm::vec3(0.0f, 0.0f, 1.0f)),
                      glm::angleAxis(glm::radians(45.0f),
                                     glm::vec3(0.0f, 0.0f, 1.0f)),
                      glm::angleAxis(glm::radians(-20.0f),
                                     glm::vec3(0.0f, 0.0f, 1.0f)),
                  }),
          },
      .durationSeconds = 1.0f,
  });

  return skeleton;
}

struct AnimatedCharacter {
  std::string name;
  SkeletonAssetData skeleton;
  ModelAnimationState animationState;
  int weaponSocketNodeIndex = 1;

  explicit AnimatedCharacter(std::string characterName)
      : name(std::move(characterName)), skeleton(buildCharacterSkeleton()) {
    animationState.reset(&skeleton);
  }

  void playClip(int animationIndex) {
    animationState.selectSourceAnimation(&skeleton, animationIndex);
    animationState.playSelectedAnimation(&skeleton);
  }

  void tick(float deltaSeconds) {
    animationState.updatePlayback(&skeleton, deltaSeconds);
  }

  const AnimationPlaybackState &playback() const {
    const AnimationPlaybackState *state = animationState.currentPlayback(&skeleton);
    if (state == nullptr) {
      throw std::runtime_error("animation playback state is not available");
    }
    return *state;
  }

  const SkeletonPose &pose() const {
    const SkeletonPose *currentPose = animationState.currentPose(&skeleton);
    if (currentPose == nullptr) {
      throw std::runtime_error("skeleton pose is not available");
    }
    return *currentPose;
  }

  glm::vec3 rootPosition() const {
    return translationFromMatrix(pose().worldTransform(0));
  }

  glm::vec3 weaponSocketPosition() const {
    return translationFromMatrix(
        pose().worldTransform(static_cast<size_t>(weaponSocketNodeIndex)));
  }

  glm::vec3 weaponAimDirection() const {
    return forwardFromMatrix(
        pose().worldTransform(static_cast<size_t>(weaponSocketNodeIndex)));
  }
};

static void printFrame(const AnimatedCharacter &character, int frameIndex) {
  const glm::vec3 root = character.rootPosition();
  const glm::vec3 socket = character.weaponSocketPosition();
  const glm::vec3 aim = character.weaponAimDirection();

  std::cout << "Frame " << frameIndex << "  time="
            << std::fixed << std::setprecision(2)
            << character.playback().currentTimeSeconds << "s"
            << "  root=(" << root.x << ", " << root.y << ", " << root.z << ")"
            << "  weapon_socket=(" << socket.x << ", " << socket.y << ", "
            << socket.z << ")"
            << "  aim=(" << aim.x << ", " << aim.y << ", " << aim.z << ")\n";
}

int main() {
  try {
    AnimatedCharacter player("Player");

    std::cout << "abstracto-animation game-engine style example\n";
    std::cout << "Character: " << player.name << "\n";
    std::cout << "Clip count: " << player.skeleton.animations.size() << "\n";
    std::cout << "Selected clip: " << player.skeleton.animations[0].name << "\n";
    std::cout << "Updating animation and querying the weapon socket each frame.\n\n";

    player.playClip(0);

    constexpr float deltaSeconds = 1.0f / 6.0f;
    for (int frameIndex = 0; frameIndex < 8; ++frameIndex) {
      player.tick(deltaSeconds);
      printFrame(player, frameIndex);
    }
  } catch (const std::exception &error) {
    std::cerr << error.what() << "\n";
    return 1;
  }

  return 0;
}
