
#pragma once

#ifndef __GFE_GroupLoopedPrim_h__
#define __GFE_GroupLoopedPrim_h__

#include "GFE/GFE_GroupLoopedPrim.h"



#include "GU/GU_Snap.h"


#include "GFE/GFE_GeoFilter.h"

#include "GFE/GFE_MeshTopology.h"
#include "GFE/GFE_GroupPromote.h"
#include "GFE/GFE_GroupUnion.h"




class GFE_GroupLoopedPrim : public GFE_AttribCreateFilter {

public:
    using GFE_AttribCreateFilter::GFE_AttribCreateFilter;

    //GFE_GroupLoopedPrim(
    //    GA_Detail* const geo,
    //    const SOP_NodeVerb::CookParms* const cookparms = nullptr
    //)
    //    : GFE_AttribCreateFilter(geo, cookparms)
    //    , groupParserSeam(geo, groupParser.getGOP(), cookparms)
    //{
    //}

    //GFE_GroupLoopedPrim(
    //    const SOP_NodeVerb::CookParms& cookparms,
    //    GA_Detail* const geo
    //)
    //    : GFE_AttribCreateFilter(cookparms, geo)
    //    , groupParserSeam(cookparms, geo, groupParser.getGOP())
    //{
    //}

    void
        setComputeParm(
            const bool preFuse = false,
            const fpreal fuseDist = 1e-05,
            const exint subscribeRatio = 64,
            const exint minGrainSize = 1024
        )
    {
        setHasComputed();

        this->preFuse        = preFuse;
        this->fuseDist       = fuseDist;
        this->subscribeRatio = subscribeRatio;
        this->minGrainSize   = minGrainSize;
    }

    void
        findOrCreate(
            const GA_GroupType unsharedAttribClass = GA_GROUP_VERTEX,
            const GA_StorageClass storageClass = GA_STORECLASS_INT,
            const GA_Storage storage = GA_STORE_INVALID,
            const bool detached = false,
            const UT_StringHolder& name = "__topo_looped"
        )
    {
        if (storageClass == GA_STORECLASS_OTHER)
        {
            isGroup = true;
            getOutGroupArray().findOrCreate(unsharedAttribClass, detached, name);
            //if (unsharedAttribClass != GA_GROUP_VERTEX)
            //{
            //    getOutGroupArray().findOrCreate(GA_GROUP_VERTEX, true);
            //}
        }
        else
        {
            isGroup = false;
            getOutAttribArray().findOrCreateTuple(GFE_Type::attributeOwner_groupType(unsharedAttribClass),
                storageClass, storage, detached, name);

            //getOutGroupArray().findOrCreate(GA_GROUP_VERTEX, true);
        }
        //unsharedGroup = static_cast<GA_VertexGroup*>(getOutGroupArray().last());
    }








private:


    virtual bool
        computeCore() override
    {
        if (groupParser.isEmpty())
            return true;

        GU_DetailHandle geoOrigin_h;
        GU_Detail* geoOriginTmp = nullptr;
        if (groupUnsharedAfterFuse)
        {
            geoOriginTmp = new GU_Detail();
            geoOrigin_h.allocateAndSet(geoOriginTmp);
            geoOriginTmp->replaceWith(*geo);

            GU_Snap::PointSnapParms fuseParms;
            fuseParms.myDistance = fuseDist;
            fuseParms.myAlgorithm = GU_Snap::PointSnapParms::SnapAlgorithm::ALGORITHM_CLOSEST_POINT;
            fuseParms.myConsolidate = true;
            fuseParms.myDeleteConsolidated = true;
            fuseParms.myQPosH = geoOriginTmp->getP();
            fuseParms.myTPosH = geoOriginTmp->getP();
            GU_Snap::snapPoints(*geoOriginTmp, nullptr, fuseParms);
            //GU_Snap::snapPoints(*geoOriginTmp, static_cast<const GU_Detail*>(geo), fuseParms);
        }
        else
        {
            geoOriginTmp = static_cast<GU_Detail*>(geo);
        }

        const GA_Storage finalStorageI = GFE_Type::getPreferredStorageI(geo);

        GA_VertexGroup* unsharedGroup = GFE_VertexNextEquiv::addGroupVertexNextEquiv(geoOriginTmp, groupParser.getVertexGroup(), GA_STORE_INVALID);

        if (isGroup)
        {
            //GFE_GroupPromote::groupPromote(geoOriginTmp, unsharedGroup, unsharedAttribClass, unsharedAttribName, true);
            GFE_GroupUnion::groupUnion(getOutGroupArray()[0], unsharedGroup);
            GFE_GroupUnion::groupUnion_topoAttrib(geo, getOutGroupArray()[0], unsharedGroup);
        }
        else
        {
            GFE_AttribCombine::attribCombine(getOutAttribArray()[0], unsharedGroup);
        }

        return true;
    }




public:
    bool preFuse = false;
    fpreal fuseDist = 1e-05;

private:
    bool isGroup = false;
    GA_VertexGroup* unsharedGroup = nullptr;

    exint subscribeRatio = 64;
    exint minGrainSize = 1024;
};











//
//
//
//
//namespace GFE_GroupLoopedPrim_Namespace {
//
//
//
//
//
//    static GA_Group*
//        groupUnshared(
//            GA_Detail* const geo,
//            const GA_VertexGroup* const geoVtxGroup,
//            const UT_StringHolder& unsharedAttribName = "unshared",
//            const GA_StorageClass unsharedAttribStorageClass = GA_STORECLASS_REAL,
//            const GA_GroupType unsharedAttribClass = GA_GROUP_VERTEX,
//            const bool groupUnsharedAfterFuse = false,
//            const GA_Storage inStorageI = GA_STORE_INVALID,
//            const bool outTopoAttrib = false,
//            const exint subscribeRatio = 64,
//            const exint minGrainSize = 64
//        )
//    {
//        UT_ASSERT_P(geo);
//
//        const GA_Storage finalStorageI = GFE_Type::getPreferredStorageI(geo);
//
//        //GA_VertexGroup* const unsharedGroup = GFE_VertexNextEquiv::addGroupVertexNextEquiv(geo, geoVtxGroup, inStorageI, "__topo_unshared_SOP_FeE_GroupUnshared_1_0");
//        GA_VertexGroup* unsharedGroup = GFE_VertexNextEquiv::addGroupVertexNextEquiv(geo, geoVtxGroup, inStorageI, "__topo_unshared", subscribeRatio, minGrainSize);
//        GA_Group* const unshared_promoGroup = GFE_GroupPromote::groupPromote(geo, unsharedGroup, unsharedAttribClass, unsharedAttribName, true);
//        return unshared_promoGroup;
//    }
//
//
//
//
//    static GA_Group*
//        groupUnshared(
//            const GA_Detail* const geo,
//            const GA_Group* const geoGroup,
//            const UT_StringHolder& unsharedAttribName = "unshared",
//            const GA_StorageClass unsharedAttribStorageClass = GA_STORECLASS_REAL,
//            const GA_GroupType unsharedAttribClass = GA_GROUP_VERTEX,
//            const bool groupUnsharedAfterFuse = false,
//            const GA_Storage inStorageI = GA_STORE_INVALID,
//            const bool outTopoAttrib = false,
//            const exint subscribeRatio = 64,
//            const exint minGrainSize = 64
//        )
//    {
//        UT_ASSERT_P(geo);
//
//        if (geoGroup && geoGroup->get == )
//        {
//            return groupUnshared(cookparms, geo, groupType, sopparms.getGroup()
//                geo0AttribNames
//                unsharedAttribStorageClass, unsharedAttribClass
//                groupUnsharedAfterFuse, sopparms.getOutTopoAttrib(),
//                subscribeRatio, minGrainSize
//            );
//        }
//        const GA_VertexGroupUPtr geoVtxGroupUPtr = GFE_GroupPromote::groupPromoteVertexDetached(geo, geoGroup);
//        //GA_VertexGroup* geoVtxGroup = static_cast<GA_VertexGroup*>(geoVtxGroupUPtr.get());
//        const GA_VertexGroup* const geoVtxGroup = geoVtxGroupUPtr.get();
//
//
//        return groupUnshared(geo, geoVtxGroup,
//            geo0AttribNames
//            unsharedAttribStorageClass, unsharedAttribClass
//            groupUnsharedAfterFuse, sopparms.getOutTopoAttrib(),
//            subscribeRatio, minGrainSize
//        );
//
//
//    }
//
//
//
//
//
//static GA_Group*
//    groupUnshared(
//    const SOP_NodeVerb::CookParms& cookparms,
//    const GA_Detail* const geo,
//    const GA_GroupType groupType,
//    const UT_StringHolder& groupName,
//    const UT_StringHolder& unsharedAttribName = "unshared",
//    const GA_StorageClass unsharedAttribStorageClass = GA_STORECLASS_REAL,
//    const GA_GroupType unsharedAttribClass = GA_GROUP_VERTEX,
//    const bool groupUnsharedAfterFuse = false,
//    const GA_Storage inStorageI = GA_STORE_INVALID,
//    const bool outTopoAttrib = false,
//    const exint subscribeRatio = 64,
//    const exint minGrainSize = 64
//    )
//{
//    UT_ASSERT_P(geo);
//    UT_ASSERT_P(group);
//
//
//    GOP_Manager gop;
//    const GA_Group* const geoGroup = GFE_GroupParser_Namespace::findOrParseGroupDetached(cookparms, geo, groupType, sopparms.getGroup(), gop);
//    if (geoGroup && geoGroup->isEmpty())
//        return;
//
//    groupUnshared(cookparms, geo, groupType, sopparms.getGroup(),
//        geo0AttribNames,
//        unsharedAttribStorageClass, unsharedAttribClass
//        groupUnsharedAfterFuse, sopparms.getOutTopoAttrib(),
//        subscribeRatio, minGrainSize
//    );
//
//
//    GFE_TopologyReference::outTopoAttrib(geo, outTopoAttrib);
//
//    if (geoGroup)
//    {
//        cookparms.getNode()->setHighlight(true);
//        cookparms.select(*unshared_promoGroup);
//    }
//    return unsharedGroup;
//}
//
//
//
//} // End of namespace GFE_GroupLoopedPrim
//
//
//
//



#endif