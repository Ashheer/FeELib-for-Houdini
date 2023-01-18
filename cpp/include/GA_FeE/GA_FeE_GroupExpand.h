
#pragma once

#ifndef __GA_FeE_GroupExpand_h__
#define __GA_FeE_GroupExpand_h__

//#include "GA_FeE/GA_FeE_GroupExpand.h"

#include "GA/GA_Detail.h"

#include "GA_FeE/GA_FeE_Group.h"
#include "GA_FeE/GA_FeE_GroupUnion.h"
#include "GA_FeE/GA_FeE_Adjacency.h"


namespace GA_FeE_GroupExpand {




    static void
        edgeGroupExpandEdge(
            GA_Detail* const geo,
            GA_EdgeGroup* const expandGroup,
            const GA_EdgeGroup* const baseGroup,
            const exint subscribeRatio = 64,
            const exint minGrainSize = 1024
        )
    {
        if (!baseGroup || baseGroup->isEmpty())
            return;

        //GA_SplittableRange geoSplittableRange(GA_FeE_Group::getRangeByAnyGroup(geo, group));
        //UTparallelFor(geoSplittableRange, [geo, group, &nebsAttrib_h](const GA_SplittableRange& r)
        //{
        //    UT_ValArray<GA_Offset> adjElems;
        //    GA_Offset start, end;
        //    for (GA_Iterator it(r); it.blockAdvance(start, end); )
        //    {
        //        for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
        //        {
        //            nebsAttrib_h.get(elemoff, adjElems);
        //            for (GA_Size i = 0; i < adjElems.size(); ++i)
        //            {
        //                group->setElement(adjElems[i], true);
        //            }
        //        }
        //    }
        //}, subscribeRatio, minGrainSize);
    }



    static void
        groupExpand(
            GA_Detail* const geo,
            GA_Group* const expandGroup,
            const GA_Group* const baseGroup,
            const GA_GroupType connectivityGroupType,
            const exint subscribeRatio = 64,
            const exint minGrainSize = 1024
        )
    {
        if (!baseGroup || baseGroup->isEmpty())
        {
            expandGroup->addAll();
            return;
        }

        const GA_GroupType groupType = baseGroup->classType();

        GA_FeE_GroupUnion::groupUnion(geo, expandGroup, baseGroup);

        if (groupType == GA_GROUP_EDGE)
        {
            //const GA_EdgeGroup* baseEdgeGroup = UTverify_cast<const GA_EdgeGroup*>(baseGroup);
            switch (connectivityGroupType)
            {
            case GA_GROUP_PRIMITIVE:
                break;
            case GA_GROUP_POINT:
                break;
            case GA_GROUP_VERTEX:
                break;
            case GA_GROUP_EDGE:
                edgeGroupExpandEdge(geo,
                    UTverify_cast<GA_EdgeGroup*>(expandGroup),
                    UTverify_cast<const GA_EdgeGroup*>(baseGroup),
                    subscribeRatio, minGrainSize);
                break;
            default:
                break;
            }
        }
        else
        {
            GA_ElementGroup* const expandElemGroup = UTverify_cast<GA_ElementGroup*>(expandGroup);

            const GA_Attribute* const nebsAttrib = GA_FeE_Adjacency::addAttribAdjacency(geo, groupType, connectivityGroupType);
            if (!nebsAttrib)
                return;
            GA_ROHandleT<UT_ValArray<GA_Offset>> nebsAttrib_h(nebsAttrib);

            GA_SplittableRange geoSplittableRange(GA_FeE_Group::getRangeByAnyGroup(geo, baseGroup));
            UTparallelFor(geoSplittableRange, [expandElemGroup, &nebsAttrib_h](const GA_SplittableRange& r)
            {
                UT_ValArray<GA_Offset> adjElems(32);
                GA_Offset start, end;
                for (GA_Iterator it(r); it.blockAdvance(start, end); )
                {
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        nebsAttrib_h.get(elemoff, adjElems);
                        for (int i = 0; i < adjElems.size(); ++i)
                        {
                            expandElemGroup->setElement(adjElems[i], true);
                        }
                    }
                }
            }, subscribeRatio, minGrainSize);
        }

    }











    static void
        edgeGroupExpandEdge(
            GA_Detail* const geo,
            GA_EdgeGroup* const expandGroup,
            GA_EdgeGroup* const borderGroup,
            GA_EdgeGroup* const prevBorderGroup,
            const GA_EdgeGroup* baseGroup,
            const exint numiter,
            const exint subscribeRatio = 64,
            const exint minGrainSize = 1024
        )
    {
        if (numiter == 0)
            return;

        for (exint iternum = 0; iternum < numiter; iternum++)
        {
            //GA_SplittableRange geoSplittableRange(GA_FeE_Group::getRangeByAnyGroup(geo, group));
            //UTparallelFor(geoSplittableRange, [geo, group, &nebsAttrib_h](const GA_SplittableRange& r)
            //{
            //    UT_ValArray<GA_Offset> adjElems;
            //    GA_Offset start, end;
            //    for (GA_Iterator it(r); it.blockAdvance(start, end); )
            //    {
            //        for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
            //        {
            //            nebsAttrib_h.get(elemoff, adjElems);
            //            for (GA_Size i = 0; i < adjElems.size(); ++i)
            //            {
            //                group->setElement(adjElems[i], true);
            //            }
            //        }
            //    }
            //}, subscribeRatio, minGrainSize);
        }
    }



    static void
    groupExpand(
        GA_Detail* const geo,
        GA_Group* const expandGroup,
        GA_Group* const borderGroup,
        GA_Group* const prevBorderGroup,
        const GA_Group* baseGroup,
        const GA_GroupType connectivityGroupType,
        const exint numiter,
        const exint subscribeRatio = 64,
        const exint minGrainSize = 1024
    )
    {
        if (numiter == 0 || !baseGroup || baseGroup->isEmpty())
            return;

        if (numiter == 1)
        {
            groupExpand(geo, expandGroup, baseGroup, connectivityGroupType, subscribeRatio, minGrainSize);
            return;
        }

        const GA_GroupType groupType = baseGroup->classType();

        UT_ASSERT_P(groupType == expandGroup->classType() && groupType == borderGroup->classType() && groupType == prevBorderGroup->classType());
        
        //GA_FeE_GroupBoolean::groupUnion(geo, expandGroup, baseGroup);
        //GA_FeE_GroupBoolean::groupUnion(geo, borderGroup, baseGroup);
        if (groupType == GA_GROUP_EDGE)
        {
            GA_FeE_GroupUnion::edgeGroupUnion(geo, expandGroup, baseGroup);
            GA_FeE_GroupUnion::edgeGroupUnion(geo, borderGroup, baseGroup);
            //const GA_EdgeGroup* baseEdgeGroup = UTverify_cast<const GA_EdgeGroup*>(baseGroup);
            switch (connectivityGroupType)
            {
            case GA_GROUP_PRIMITIVE:
                break;
            case GA_GROUP_POINT:
                break;
            case GA_GROUP_VERTEX:
                break;
            case GA_GROUP_EDGE:
                edgeGroupExpandEdge(geo,
                    UTverify_cast<GA_EdgeGroup*>(expandGroup),
                    UTverify_cast<GA_EdgeGroup*>(borderGroup),
                    UTverify_cast<GA_EdgeGroup*>(prevBorderGroup),
                    UTverify_cast<const GA_EdgeGroup*>(baseGroup),
                    numiter, subscribeRatio, minGrainSize);
                break;
            default:
                break;
            }
        }
        else
        {
            GA_ElementGroup* expandElemGroup     = UTverify_cast<GA_ElementGroup*>(expandGroup);
            GA_ElementGroup* borderElemGroup     = UTverify_cast<GA_ElementGroup*>(borderGroup);
            GA_ElementGroup* prevBorderElemGroup = UTverify_cast<GA_ElementGroup*>(prevBorderGroup);
            expandElemGroup->combine(baseGroup);
            borderElemGroup->combine(baseGroup);

            GA_Attribute* nebsAttrib = GA_FeE_Adjacency::addAttribAdjacency(geo, groupType, connectivityGroupType);
            if (!nebsAttrib)
                return;
            GA_RWHandleT<UT_ValArray<GA_Offset>> nebsAttrib_h(nebsAttrib);
            for (int iternum = 0; iternum < numiter; iternum++)
            {
                //GEO_FeE_Group::groupUnion(geo, prevBorderGroup, borderGroup);
                prevBorderGroup->combine(borderGroup);
                GA_SplittableRange geoSplittableRange(GA_FeE_Group::getRangeByAnyGroup(geo, prevBorderElemGroup));
                UTparallelFor(geoSplittableRange, [geo, expandElemGroup, borderElemGroup, prevBorderElemGroup, &nebsAttrib_h](const GA_SplittableRange& r)
                {
                    UT_ValArray<GA_Offset> adjElems(32);
                    GA_Offset start, end;
                    for (GA_Iterator it(r); it.blockAdvance(start, end); )
                    {
                        for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                        {
                            borderElemGroup    ->setElement(elemoff, false);
                            prevBorderElemGroup->setElement(elemoff, false);

                            nebsAttrib_h.get(elemoff, adjElems);
                            for (int i = 0; i < adjElems.size(); ++i)
                            {
                                if (expandElemGroup->contains(adjElems[i]))
                                    continue;
                                borderElemGroup->setElement(adjElems[i], true);
                                
                            }
                        }
                    }
                }, subscribeRatio, minGrainSize);
                expandElemGroup->combine(borderElemGroup);
            }
        }

    }




    //static void
    //    pointGroupExpandEdge(
    //        GA_Detail* const geo,
    //        GA_PointGroup* const group,
    //        const exint numiter,
    //        const exint subscribeRatio = 64,
    //        const exint minGrainSize = 1024
    //    )
    //{
    //    if (numiter == 0)
    //        return;

    //    GA_RWHandleT<UT_ValArray<GA_Offset>> nebsAttrib_h = GA_FeE_Adjacency::addAttribPointPointEdge(geo);

    //    for (exint iternum = 0; iternum < numiter; iternum++)
    //    {
    //        GA_SplittableRange geoSplittableRange(GA_FeE_Group::getRangeByAnyGroup(geo, group));
    //        UTparallelFor(geoSplittableRange, [geo, group, &nebsAttrib_h](const GA_SplittableRange& r)
    //        {
    //            UT_ValArray<GA_Offset> adjElems;
    //            GA_Offset start, end;
    //            for (GA_Iterator it(r); it.blockAdvance(start, end); )
    //            {
    //                for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
    //                {
    //                    nebsAttrib_h.get(elemoff, adjElems);
    //                    for (GA_Size i = 0; i < adjElems.size(); ++i)
    //                    {
    //                        group->setElement(adjElems[i], true);
    //                    }
    //                }
    //            }
    //        }, subscribeRatio, minGrainSize);
    //    }
    //}


    //static void
    //groupExpand1(
    //    GA_Detail* const geo,
    //    GA_Group* const group,
    //    const GA_GroupType connectivityGroupType,
    //    const exint numiter,
    //    const exint subscribeRatio = 64,
    //    const exint minGrainSize = 1024
    //)
    //{
    //    if (numiter == 0)
    //        return;

    //    GA_GroupType groupType = group->classType();
    //    switch (groupType)
    //    {
    //    case GA_GROUP_PRIMITIVE:
    //        break;
    //    case GA_GROUP_POINT:
    //        switch (connectivityGroupType)
    //        {
    //        case GA_GROUP_PRIMITIVE:
    //            break;
    //        case GA_GROUP_POINT:
    //            break;
    //        case GA_GROUP_VERTEX:
    //            break;
    //        case GA_GROUP_EDGE:
    //            pointGroupExpandEdge(geo, UTverify_cast<GA_PointGroup*>(group), numiter, subscribeRatio, minGrainSize);
    //            break;
    //        default:
    //            break;
    //        }


    //        break;
    //    case GA_GROUP_VERTEX:
    //        break;
    //    case GA_GROUP_EDGE:
    //        break;
    //    default:
    //        break;
    //    }

    //}








} // End of namespace GA_FeE_GroupExpand

#endif
