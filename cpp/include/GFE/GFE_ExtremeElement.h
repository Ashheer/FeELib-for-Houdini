
#pragma once

#ifndef __GFE_ExtremeElement_h__
#define __GFE_ExtremeElement_h__

#include "GFE/GFE_ExtremeElement.h"



#include "GFE/GFE_GeoFilter.h"


#include "GFE/GFE_Measure.h"


class GFE_ExtremeElement : public GFE_AttribCreateFilterWithRef0 {

#define __GFE_ExtremeElement_UseMeasureAttrib__ 0


public:
    
#if __GFE_ExtremeElement_UseMeasureAttrib__
    GFE_ExtremeElement(
        GA_Detail& geo,
        const GA_Detail* const geoRef0,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
        : GFE_AttribCreateFilterWithRef(geo, cookparms)
        , measure(geo, cookparms)
    {
    }
#else
    using GFE_AttribCreateFilterWithRef0::GFE_AttribCreateFilterWithRef0;
#endif
    
    void
        setComputeParm(
            const GFE_StatisticalFunction statisticalFunction = GFE_StatisticalFunction::Min,
            const bool delExtremeAttrib = false,
            const exint subscribeRatio = 64,
            const exint minGrainSize   = 1024
        )
    {
        setHasComputed();
        this->statisticalFunction = statisticalFunction;
        this->delExtremeAttrib    = delExtremeAttrib;
        this->subscribeRatio = subscribeRatio;
        this->minGrainSize   = minGrainSize;
    }

    
    SYS_FORCE_INLINE GA_Attribute* setExtremeElemAttrib(const bool detached = false, const GA_Storage storage = GA_STORE_INVALID, const UT_StringRef& attribName = "")
    { return extremeElemAttrib = getOutAttribArray().findOrCreateTuple(detached, GA_ATTRIB_DETAIL, GA_STORECLASS_INT, GA_STORE_INVALID, attribName); }
    
    SYS_FORCE_INLINE GA_Attribute* setExtremeElemAttrib(const bool detached = false, const UT_StringRef& attribName = "")
    { return setExtremeElemAttrib(detached, GA_STORE_INVALID, attribName); }
    
    //SYS_FORCE_INLINE GA_Attribute* setExtremeValueAttrib(const bool detached = false, const GA_Storage storage = GA_STORE_INVALID, const UT_StringRef& attribName = "")
    //{ return extremeValueAttrib = getOutAttribArray().findOrCreateTuple(detached, GA_ATTRIB_DETAIL, GA_STORECLASS_INT, GA_STORE_INVALID, attribName); }
    //
    //SYS_FORCE_INLINE GA_Attribute* setExtremeValueAttrib(const bool detached = false, const UT_StringRef& attribName = "")
    //{ return setExtremeValueAttrib(detached, GA_STORE_INVALID, attribName); }
    
    SYS_FORCE_INLINE void setExtremeValueAttrib()
    { extremeValueAttribName = nullptr; }
    
    SYS_FORCE_INLINE void setExtremeValueAttrib(const bool detached = false, const UT_StringRef& attribName)
    { extremeValueAttribDetached = detached; extremeValueAttribName = &attribName; }
    
    
    SYS_FORCE_INLINE GA_Offset getExtremeElemoff() const
    { return extremeElemoff; }
    

    
private:


    virtual bool
        computeCore() override
    {
        if (getInAttribArray().isEmpty() || getOutGroupArray().isEmpty())
            return false;

        if (groupParser.isEmpty())
            return true;


#if __GFE_ExtremeElement_UseMeasureAttrib__
        {
            measure.groupParser.setGroup(groupParser);
            extremeAttrib = measure.findOrCreateTuple(false);
            measure.compute();
        }
#else
        {
            extremeAttribNonConst = getInAttribArray()[0];
            extremeAttrib = extremeAttribNonConst;
        }
#endif
        
        const GA_Size numElems = geo->getNumElements(extremeAttrib);
        
        if (numElems <= 1)
        {
            if (numElems == 1)
                extremeElemoff = 0;
            else
                extremeElemoff = GFE_INVALID_OFFSET;
            
            if (delExtremeAttrib && extremeAttribNonConst)
                geo->destroyAttrib(extremeAttribNonConst);
            return true;
        }
        
        switch (extremeAttrib->getAIFTuple()->getStorage(extremeAttrib))
        {
        case GA_STORE_INT16:  computeExtremeElemoff<int16>();    break;
        case GA_STORE_INT32:  computeExtremeElemoff<int32>();    break;
        case GA_STORE_INT64:  computeExtremeElemoff<int64>();    break;
        case GA_STORE_REAL16: computeExtremeElemoff<fpreal16>(); break;
        case GA_STORE_REAL32: computeExtremeElemoff<fpreal32>(); break;
        case GA_STORE_REAL64: computeExtremeElemoff<fpreal64>(); break;
        }

        //if (extremeValueAttrib)
        //{
        //    const GA_AIFTuple* const aIFTuple = extremeValueAttrib->getAIFTuple();
        //    UT_ASSERT_P(aIFTuple);
        //    aIFTuple->set(extremeValueAttrib, 0, extremeValueAttrib);
        //}
        
        if (extremeElemAttrib)
        {
            const GA_AIFTuple* const aIFTuple = extremeElemAttrib->getAIFTuple();
            UT_ASSERT_P(aIFTuple);
            aIFTuple->set(extremeElemAttrib, 0, extremeElemoff);
        }
        
        if (!getOutGroupArray().isEmpty())
        {
            GA_ElementGroup* const outGroup = getOutGroupArray().getElementGroup(0);
            if (outGroup->getOwner() == extremeAttrib->getOwner())
            {
                outGroup->setElement(extremeElemoff, true);
            }
        }

        if (delExtremeAttrib && extremeAttribNonConst)
            geo->destroyAttrib(extremeAttribNonConst);
    
        
        return true;
    }



    
    template<typename FLOAT_T>
    void computeExtremeElemoff()
    {
        if (extremeValueAttribName)
        {
            GA_Attribute* const extremeValueAttrib = getOutAttribArray().findOrCreateTuple();
            const GA_AIFTuple* const aIFTuple = extremeValueAttrib->getAIFTuple();
            UT_ASSERT_P(aIFTuple);
            aIFTuple->set(extremeValueAttrib, 0, extremeValueAttrib);
        }
        
        
        ComputeExtremeElemoff<FLOAT_T> body(geo, extremeAttrib, statisticalFunction);
        const GA_SplittableRange geoSplittableRange(geo->getPrimitiveRange(groupParser.getPrimitiveGroup()));
        UTparallelReduce(geoSplittableRange, body, subscribeRatio, minGrainSize);
        extremeElemoff = body.getExtremeElemoff();
    }
    
    
    template<typename T>
    class ComputeExtremeElemoff
    {
    public:
        ComputeExtremeElemoff(const GFE_Detail* const a, const GA_Attribute* b, const GFE_StatisticalFunction c)
            : myGeo(a)
            , myAttrib(b)
            , myStatisticalFunction(c)
            , myExtremeElemoff(GA_INVALID_OFFSET)
            , myExtremeValue(myStatisticalFunction == GFE_StatisticalFunction::Max ? SYS_FP64_MIN : SYS_FP64_MAX)
        {}
        ComputeExtremeElemoff(ComputeExtremeElemoff& src, UT_Split)
            : myGeo(src.myGeo)
            , myAttrib(src.myAttrib)
            , myStatisticalFunction(src.myStatisticalFunction)
            , myExtremeElemoff(GA_INVALID_OFFSET)
            , myExtremeValue(myStatisticalFunction == GFE_StatisticalFunction::Max ? SYS_FP64_MIN : SYS_FP64_MAX)
        {}
        void operator()(const GA_SplittableRange& r)
        {
            GA_PageHandleT<T, T, true, false, const GA_Attribute, const GA_ATINumeric, const GA_Detail> attrib_ph(myAttrib);
            for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
            {
                GA_Offset start, end;
                for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
                {
                    attrib_ph.setPage(start);
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        switch (myStatisticalFunction)
                        {
                        case GFE_StatisticalFunction::Min:
                            if (attrib_ph.value(elemoff) < myExtremeValue) 
                                myExtremeValue = attrib_ph.value(elemoff);
                            break;
                        case GFE_StatisticalFunction::Max:
                            if (attrib_ph.value(elemoff) > myExtremeValue)
                                myExtremeValue = attrib_ph.value(elemoff);
                            break;
                        default: break;
                        }
                    }
                }
            }
        }
        void join(const ComputeExtremeElemoff& other)
        {
            myExtremeElemoff = other.myExtremeElemoff;
            myExtremeValue = other.myExtremeValue;
        }
        SYS_FORCE_INLINE GA_Offset getExtremeElemoff() const
        { return myExtremeElemoff; }
        SYS_FORCE_INLINE T getExtremeValue() const
        { return myExtremeValue; }
    private:
        const GFE_Detail* const myGeo;
        const GA_Attribute* myAttrib;
        GFE_StatisticalFunction myStatisticalFunction;
        GA_Offset myExtremeElemoff;
        T myExtremeValue;
    }; // End of Class ComputeExtremeElemoff


public:
    
#if __GFE_ExtremeElement_UseMeasureAttrib__
    GFE_Measure measure;
#endif
    
    GFE_StatisticalFunction statisticalFunction = GFE_StatisticalFunction::Min;
    bool delExtremeAttrib = false;
    
    //const UT_StringRef* extremeElemGroupName = nullptr;
    bool extremeValueAttribDetached = false;
    const UT_StringRef* extremeValueAttribName = nullptr;
private:
    GA_Offset extremeElemoff = GFE_INVALID_OFFSET;
    const GA_Attribute* extremeAttrib;
    GA_Attribute* extremeAttribNonConst;
    
    GA_Attribute* extremeElemAttrib = nullptr;
    //GA_Attribute* extremeValueAttrib = nullptr;

    
    exint subscribeRatio = 64;
    exint minGrainSize = 1024;
    
#undef __GFE_ExtremeElement_UseMeasureAttrib__
    
}; // End of Class GFE_ExtremeElement






#endif
