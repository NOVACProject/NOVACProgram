/**
* Contains a parameter link item object.
*
* @author		\URL[Stefan Kraus]{http://stefan@00kraus.de} @ \URL[IWR, Image Processing Group]{http://klimt.iwr.uni-heidelberg.de}
* @version		1.0 @ 2002/02/11
*/
#if !defined(PARAMETERLINKITEM_H_20020211)
#define PARAMETERLINKITEM_H_20020211

namespace MathFit
{
	// forward declaration
	class CParameterVector;

	/**
	* Represents a single parameter link defintion.
	* If parameters are linked together, for each link established an object of this type will be
	* created to hold all neccessary information about the link's state.
	*
	* @author		\URL[Stefan Kraus]{http://stefan@00kraus.de} @ \URL[IWR, Image Processing Group]{http://klimt.iwr.uni-heidelberg.de}
	* @version		1.0 @ 2002/02/11
	*/
	class CParameterLinkItem
	{
	public:
		/**
		* Constructs a new link item object.
		*
		* @param iSrcID	The global parameter ID of the link source's parameter within the current parameter object. (See \Ref{FixParameter})
		* @param pvTarget	The parameter vector object belonging to the link target.
		* @param iTargetID	The global parameter ID of the link target's parameter within the \Ref{pvTarget} parameter object. (See \Ref{FixParameter})
		* @param pPrev		A pointer to the previous link item object. Needed to build a chain of link items.
		* @param pNext		A pointer to the next link item object. Needed to build a chain of link items.
		*/
		CParameterLinkItem(int iSrcID, CParameterVector* pvTarget, int iTargetID, CParameterLinkItem* pPrev = NULL, CParameterLinkItem* pNext = NULL) : mTarget(pvTarget), mTargetID(iTargetID), mSrcID(iSrcID), mPrev(pPrev), mNext(pNext)
		{
		}

		/**
		* Returns the next link item in the current chain.
		*
		* @return	A pointer to the next item. If no item follows this item, NULL will be returned.
		*/
		CParameterLinkItem* GetNextItem()
		{
			return mNext;
		}

		/**
		* Sets the next link item in the current chain.
		*
		* @param pNext	A pointer to the next link item.
		*/
		void SetNextItem(CParameterLinkItem* pNext)
		{
			mNext = pNext;
		}

		/**
		* Returns the previous link item in the current chain.
		*
		* @return	A pointer to the previous item. If no item preceeds this one, NULL is returned.
		*/
		CParameterLinkItem* GetPreviousItem()
		{
			return mPrev;
		}

		/**
		* Sets the previous link item in the current chain.
		*
		* @param pPrev	A pointer to the previous link item.
		*/
		void SetPreviousItem(CParameterLinkItem* pPrev)
		{
			mPrev = pPrev;
		}

		/**
		* Returns the parameter ID of the link's source parameter.
		*
		* @return	The parameter ID of the link's source parameter.
		*/
		int GetSrcID()
		{
			return mSrcID;
		}

		/**
		* Sets the link's source parameter ID.
		*
		* @param iSrcID	The parameter ID of the link's source parameter.
		*/
		void SetSrcID(int iSrcID)
		{
			mSrcID = iSrcID;
		}

		/**
		* Returns the link target's parameter vector object.
		*
		* @return	A pointer to the link target's parameter vector object.
		*/
		CParameterVector* GetTargetObj()
		{
			return mTarget;
		}

		/**
		* Sets the link target's parameter vector object.
		*
		* @param pvTarget	Pointer to the link target's parameter vector object which holds the link target's parameter.
		*/
		void SetTargetObj(CParameterVector* pvTarget)
		{
			mTarget = pvTarget;
		}

		/**
		* Returns the link target's parameter ID.
		*
		* @return	The parameter ID within the link target's parameter vector of the link target.
		*/
		int GetTargetID()
		{
			return mTargetID;
		}

		/**
		* Sets the parameter ID of the link target.
		*
		* @param iTargetID	The parameter ID of the link target. This is a global parameter ID within the link target's parameter vector object.
		*/
		void SetTargetID(int iTargetID)
		{
			mTargetID = iTargetID;
		}

		/**
		* Checks wheter two link item objects are equal.
		*
		* @param pOp	Pointer to another link item object.
		*
		* @return	TRUE if the current object has the same link parameters as the given link item object. FALSE otherwise.
		*/
		bool IsEqual(CParameterLinkItem* pOp)
		{
			return mSrcID == pOp->GetSrcID() && mTarget == pOp->GetTargetObj() && mTargetID == pOp->GetTargetID();
		}

		/**
		* Checks wheter given link parameters match the current link parameters.
		*
		* @param iSrcID	The parameter ID of the link source.
		* @param pvTarget	The parameter vector object of the link target.
		*
		* @return	TRUE if the current object has a link from the source parameter to the parameter vector object. FALSE otherwise.
		*/
		bool IsLinked(int iSrcID, CParameterVector* pvTarget)
		{
			return mSrcID == iSrcID && mTarget == pvTarget;
		}

	private:
		/**
		* Holds the link target's parameter vector object.
		*/
		CParameterVector* mTarget;
		/**
		* Holds the parameter ID of the link target.
		*/
		int mTargetID;
		/**
		* Holds the parameter ID of the link source.
		*/
		int mSrcID;
		/**
		* Holds a pointer to the next link item in the current chain.
		*/
		CParameterLinkItem* mNext;
		/**
		* Holds a pointer to the previous link item in the current chain.
		*/
		CParameterLinkItem* mPrev;
	};
}
#endif