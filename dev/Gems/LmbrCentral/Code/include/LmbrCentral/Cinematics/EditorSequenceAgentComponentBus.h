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
#pragma once

#include <AzCore/Component/ComponentBus.h>
#include <AzCore/EBus/EBus.h>
#include <AzCore/std/string/string.h>
#include <LmbrCentral/Cinematics/SequenceComponentBus.h>
#include <IMovieSystem.h>

#include "SequenceAgentComponentBus.h"

namespace LmbrCentral
{
    /*!
    * EditorSequenceAgentComponentRequests EBus Interface
    * Messages serviced by EditorSequenceAgentComponents.
    *
    * The EBus is Id'ed on a pair of SequenceEntityId, SequenceAgentEntityId 
    */

    class EditorSequenceAgentComponentBus
        : public AZ::EBusTraits
    {
    public:
        virtual ~EditorSequenceAgentComponentBus() = default;

        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::ById;
        using BusIdType = SequenceAgentEventBusId;
        //////////////////////////////////////////////////////////////////////////
    };

    class EditorSequenceAgentComponentRequests
        : public EditorSequenceAgentComponentBus
    {
    public:

        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides - application is a singleton
        static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;  // Only one component on a entity can implement the events
        //////////////////////////////////////////////////////////////////////////

        //! Returns a list of all animatable properties for a given componentId on the entity which holds the SequenceAgent Component
        virtual void GetAllAnimatableProperties(IAnimNode::AnimParamInfos& propertyNames, AZ::ComponentId componentId) = 0;

        //! Append all animatable components on the entity which holds the SequenceAgent Component
        virtual void GetAnimatableComponents(AZStd::vector<AZ::ComponentId>& animatableComponentIds) = 0;
    };

    using EditorSequenceAgentComponentRequestBus = AZ::EBus<EditorSequenceAgentComponentRequests>;
} // namespace LmbrCentral