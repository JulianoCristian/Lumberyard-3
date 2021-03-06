/*
* All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
* its licensors.
*
* For complete copyright and license terms please see the LICENSE at the root of this
* distribution (the "License"). All use of this software is governed by the License,
* or, if provided, by the license below or the license accompanying this file. Do not
* remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*
*/
// Original file Copyright Crytek GMBH or its affiliates, used under license.

#pragma once

#include <Cry_Math.h>
#include <Cry_Geo.h>
#include <Cry_Camera.h>
#include <smartptr.h>
#include <boost/shared_ptr.hpp>

#include <QString>
#include <QObject>
#include <vector>
#include <string>

#include <CryCharAnimationParams.h>
#include "Serialization/Strings.h"
#include "../EditorCommon/QViewportEvents.h"
#include "Explorer.h" // for selected explorer entries
#include "PlaybackLayers.h"

#define DEFAULT_FRAME_RATE ANIMATION_30Hz

namespace Serialization
{
    class IArchive;
    struct SStruct;
    class CContextList;
    struct SContextLink;
}

struct ICharacterInstance;
struct IDefaultSkeleton;
struct ICharacterManager;
struct IPhysicalEntity;
struct IMaterial;
struct IAnimationGroundAlignment;
struct IStatObj;
struct SRendParams;
struct SRenderingPassInfo;
struct SRenderContext;
struct IShaderPublicParams;
struct IAttachmentSkin;
struct IRenderAuxGeom;
struct SViewportState;
namespace Serialization
{
    struct INavigationProvider;
}
class CDLight;

namespace CharacterTool {
    using std::vector;
    using std::unique_ptr;
    using Serialization::string;
    using Serialization::IArchive;

    struct AnimationSetFilter;
    class CompressionMachine;
    struct CharacterDefinition;
    struct DisplayOptions;

    class EffectPlayer;

    struct PlaybackOptions
    {
        bool loopAnimation;
        bool playOnAnimationSelection;
        bool playFromTheStart;
        bool firstFrameAtEndOfTimeline;
        bool wrapTimelineSlider;
        bool smoothTimelineSlider;
        float playbackSpeed;

        PlaybackOptions();
        void Serialize(IArchive& ar);

        bool operator!=(const PlaybackOptions& rhs) const
        {
            return loopAnimation != rhs.loopAnimation ||
                playOnAnimationSelection != rhs.playOnAnimationSelection ||
                   playFromTheStart != rhs.playFromTheStart ||
                   firstFrameAtEndOfTimeline != rhs.firstFrameAtEndOfTimeline ||
                   wrapTimelineSlider != rhs.wrapTimelineSlider ||
                   smoothTimelineSlider != rhs.smoothTimelineSlider ||
                   playbackSpeed != rhs.playbackSpeed;
        }
    };

    struct ViewportOptions
    {
        float lightMultiplier;
        float lightSpecMultiplier;
        float lightRadius;
        float lightOrbit;
        Vec3 lightDiffuseColor0;
        Vec3 lightDiffuseColor1;
        Vec3 lightDiffuseColor2;
        bool enableLighting;
        bool animateLights;

        ViewportOptions();
        void Serialize(IArchive& ar);
    };


    struct AnimationDrivenSamples
    {
        enum
        {
            maxCount = 200
        };

        std::vector<Vec3> samples;
        int count;
        int nextIndex;
        float time;
        float updateTime;

        void Reset();
        void Add(const float fTime, const Vec3& pos);
        void Draw(IRenderAuxGeom* aux);

        AnimationDrivenSamples()
            : count(0)
            , nextIndex(0)
            , time(0)
            , updateTime(0.016666f)
        {
            samples.resize(maxCount);
        }
    };

    enum PlaybackState
    {
        PLAYBACK_UNAVAILABLE,
        PLAYBACK_PLAY,
        PLAYBACK_PAUSE
    };

    struct StateText;

    struct System;

    class CharacterDocument
        : public QObject
    {
        Q_OBJECT
    public:
        CharacterDocument(System* system);
        ~CharacterDocument();

        void ConnectExternalSignals();

        void PreRender(const SRenderContext& context);
        void Render(const SRenderContext& context);
        void PreRenderOriginal(const SRenderContext& context);
        void RenderOriginal(const SRenderContext& context);

        void LoadCharacter(const char* path);
        const char* LoadedCharacterFilename() const{ return m_loadedCharacterFilename.c_str(); }

        PlaybackOptions& GetPlaybackOptions() { return m_playbackOptions; }
        void PlaybackOptionsChanged();
        void DisplayOptionsChanged();
        CompressionMachine* GetCompressionMachine() { return m_compressionMachine.get(); }
        ICharacterInstance* CompressedCharacter() { return m_compressedCharacter.get(); }
        ICharacterInstance* UncompressedCharacter() { return m_uncompressedCharacter.get(); }
        const QuatTS& PhysicalLocation() const{ return m_PhysicalLocation; }
        void SetPhysicalLocation(const QuatTS& location) { m_PhysicalLocation = location; }

        void Serialize(IArchive& ar);

        void IdleUpdate();

        bool IsActiveInDocument(ExplorerEntry* entry) const;
        ExplorerEntry* GetActiveCharacterEntry() const;
        ExplorerEntry* GetActiveAnimationEntry() const;
        ExplorerEntry* GetActivePhysicsEntry() const;
        ExplorerEntry* GetActiveRigEntry() const;
        void GetSelectedExplorerEntries(ExplorerEntries* entries) const;
        bool IsExplorerEntrySelected(ExplorerEntry* entry) const;
        void GetSelectedExplorerActions(ExplorerActions* actions) const;
        CharacterDefinition* GetLoadedCharacterDefinition() const;
        void GetEntriesActiveInDocument(ExplorerEntries* entries) const;

        enum
        {
            SELECT_DO_NOT_REWIND = 1 << 0
        };
        void SetSelectedExplorerEntries(const ExplorerEntries& entries, int selectOptions);
        bool HasModifiedExporerEntriesSelected() const;
        bool HasSelectedExplorerEntries() const{ return !m_selectedExplorerEntries.empty(); }
        bool BindPoseEnabled() const { return m_bindPoseEnabled; }
        void SetBindPoseEnabled(bool bindPoseEnabled);

        std::shared_ptr<DisplayOptions> GetDisplayOptions() const {return m_displayOptions; }
        ViewportOptions& GetViewportOptions() { return m_viewportOptions; }

        enum
        {
            PREVIEW_ALLOW_REWIND = 1 << 0,
            PREVIEW_FORCE_RECOMPILE = 1 << 1
        };
        void TriggerAnimationPreview(int previewFlags);
        bool HasAnimationsSelected() const;

        void ScrubTime(float time, bool scrubThrough);

        float PlaybackTime() const { return m_playbackTime; }
        float PlaybackDuration() const { return m_playbackDuration; }
        float FrameRate() const { return m_frameRate; }
        PlaybackState GetPlaybackState() const;
        const char* PlaybackBlockReason() const { return m_playbackBlockReason; }
        void Play();
        void Pause();
        void EnableAudio(bool enable);
        void SyncPreviewAnimations();
        bool SetAnimEventsFile(string animEventsAssetPath);
    signals:
        void SignalPlaybackTimeChanged();
        void SignalPlaybackStateChanged();
        void SignalPlaybackOptionsChanged();
        void SignalBlendShapeOptionsChanged();

        void SignalSelectedEntriesChanged(bool continuousChange);
        void SignalAttachmentSelectionChanged();
        void SignalExplorerEntrySubSelectionChanged(ExplorerEntry* entry);

        void SignalActiveCharacterChanged();
        void SignalActiveAnimationSwitched();
        void SignalCharacterAboutToBeLoaded();
        void SignalCharacterLoaded();
        void SignalAnimationSelected();
        void SignalBindPoseModeChanged();
        void SignalAnimationStarted();
        void SignalExplorerSelectionChanged();
        void SignalDisplayOptionsChanged(const DisplayOptions& displayOptions);

    public slots:
        void OnSceneAnimEventPlayerTypeChanged();
        void OnSceneCharacterChanged();
        void OnSceneNewLayerActivated();
        void OnCharacterModified(EntryModifiedEvent& ev);
        void OnCharacterSavedAs(const char* oldName, const char* newName);
        void OnCharacterDeleted(const char* filename);
        void OnScenePlaybackLayersChanged(bool continuous);
        void OnSceneLayerActivated();
    protected slots:
        void OnExplorerSelectedEntryClicked(ExplorerEntry* e);
        void OnExplorerEntryModified(ExplorerEntryModifyEvent& ev);
        void OnExplorerEntrySavedAs(const char* oldPath, const char* newPath);
        void OnCompressionMachineAnimationStarted();
        void OnAnimationEntryRemoveReported(int subtree, unsigned int id);

    private:
        static int AnimationEventCallback(ICharacterInstance* instance, void* userData);
        void OnAnimEvent(ICharacterInstance* character);
        void PlayAnimEvent(ICharacterInstance* character, const AnimEventInstance& event);
        void TriggerAnimEventsInRange(float timeFrom, float timeTo);

        string GetDefaultSkeletonAlias();
        void ReloadCHRPARAMS();
        void ReleaseObject();
        void Physicalize();
        void CreateShaderParamCallbacks();
        void CreateShaderParamCallback(const char* name, const char* UIname, uint32 iIndex);
        void DrawCharacter(ICharacterInstance* pInstanceBase, const SRenderContext& context);
        void PreviewAnimationEntry(bool forceRecompile);
        void EnableMotionParameters();
        void UpdateBlendShapeParameterList();
        void SetPlaybackScale(float scale);

        System* m_system;

        _smart_ptr<CompressionMachine> m_compressionMachine;
        ExplorerEntries m_selectedExplorerEntries;

        float m_camRadius;
        Matrix34 m_LocalEntityMat;
        Matrix34 m_PrevLocalEntityMat;

        // per instance shader public params
        _smart_ptr<IShaderPublicParams> m_pShaderPublicParams;
        typedef std::vector<IShaderParamCallbackPtr> TCallbackVector;
        TCallbackVector m_Callbacks;
        TCallbackVector m_pShaderParamCallbackArray;

        QuatT m_lastCalculateRelativeMovement;
        float m_NormalizedTime;
        float m_NormalizedTimeSmooth;
        float m_NormalizedTimeRate;

        CCamera m_Camera;
        AABB m_AABB;
        QuatTS m_PhysicalLocation;
        float m_absCurrentSlope;
        ICharacterManager* m_characterManager;
        IPhysicalEntity* m_pPhysicalEntity;
        _smart_ptr<IMaterial>  m_pDefaultMaterial;

        string m_loadedCharacterFilename;
        string m_loadedSkeleton;
        unique_ptr<AnimationSetFilter> m_loadedAnimationSetFilter;

        _smart_ptr<ICharacterInstance> m_compressedCharacter;
        _smart_ptr<ICharacterInstance> m_uncompressedCharacter;
        vector<StateText> m_compressedStateTextCache;
        vector<StateText> m_uncompressedStateTextCache;

        boost::shared_ptr<IAnimationGroundAlignment> m_groundAlignment;
        float m_AverageFrameTime;

        ViewportOptions m_viewportOptions;
        bool m_showOriginalAnimation;
        std::shared_ptr<DisplayOptions> m_displayOptions;
        AnimationDrivenSamples m_animDrivenSamples;
        unique_ptr<SViewportState> m_viewportState;

        ICVar* m_cvar_drawEdges;
        ICVar* m_cvar_drawLocator;

        bool m_updateCameraTarget;

        bool m_bPaused;
        bool m_bindPoseEnabled;
        float m_playbackTime;
        float m_playbackDuration;
        float m_frameRate;
        PlaybackOptions m_lastScrubPlaybackOptions;
        PlaybackOptions m_playbackOptions;
        PlaybackState m_playbackState;
        const char* m_playbackBlockReason;
    };
}
