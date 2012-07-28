//////////////////////////////////////////////////////////////////////////
//  
//  Copyright (c) 2011-2012, John Haddon. All rights reserved.
//  Copyright (c) 2011-2012, Image Engine Design Inc. All rights reserved.
//  
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are
//  met:
//  
//      * Redistributions of source code must retain the above
//        copyright notice, this list of conditions and the following
//        disclaimer.
//  
//      * Redistributions in binary form must reproduce the above
//        copyright notice, this list of conditions and the following
//        disclaimer in the documentation and/or other materials provided with
//        the distribution.
//  
//      * Neither the name of John Haddon nor the names of
//        any other contributors to this software may be used to endorse or
//        promote products derived from this software without specific prior
//        written permission.
//  
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
//  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
//  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
//  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
//  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
//  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
//  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//  
//////////////////////////////////////////////////////////////////////////

#ifndef GAFFER_SCRIPTNODE_H
#define GAFFER_SCRIPTNODE_H

#include <stack>

#include "Gaffer/Node.h"
#include "Gaffer/TypedPlug.h"
#include "Gaffer/NumericPlug.h"
#include "Gaffer/Container.h"
#include "Gaffer/StandardSet.h"
#include "Gaffer/UndoContext.h"

typedef struct _object PyObject;

namespace Gaffer
{

IE_CORE_FORWARDDECLARE( Action );
IE_CORE_FORWARDDECLARE( ScriptNode );
IE_CORE_FORWARDDECLARE( ApplicationRoot );
IE_CORE_FORWARDDECLARE( Context );

typedef Container<GraphComponent, ScriptNode> ScriptContainer;
IE_CORE_DECLAREPTR( ScriptContainer );

/// The ScriptNode class represents a script - that is a single collection of
/// nodes which are stored in a single file.
/// \todo fileValid() plug or summink - bool which is true when the file is saved and
/// false when it's modified in memory. this means attaching a plugchanged callback to every
/// node (or having the Node find its parent script and set the value - I think that might be better).
class ScriptNode : public Node
{

	public :

		ScriptNode( const std::string &name=staticTypeName() );
		virtual ~ScriptNode();

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( ScriptNode, ScriptNodeTypeId, Node );
				
		/// Accepts parenting only to a ScriptContainer.
		virtual bool acceptsParent( const GraphComponent *potentialParent ) const;
		
		/// Convenience function which simply returns ancestor<ApplicationRoot>().
		ApplicationRoot *applicationRoot();
		const ApplicationRoot *applicationRoot() const;
		
		//! @name Selection
		/// The ScriptNode maintains a list of child Nodes which are considered
		/// to be selected - actions performing on the script can then use that
		/// selection any way they see fit.
		////////////////////////////////////////////////////////////////////
		//@{
		StandardSetPtr selection();
		ConstStandardSetPtr selection() const;
		//@}
		
		//! @name History and undo
		/// Certain methods in the graph API are undoable on request.
		/// These methods are implemented in terms of the Action class -
		/// when the methods are called an Action instance is stored in an
		/// undo list on the relevant ScriptNode so it can later be undone.
		/// To enable undo for a series of operations an UndoContext must
		/// be active while those operations are being performed.
		////////////////////////////////////////////////////////////////////
		//@{
		bool undoAvailable() const;
		void undo();
		bool redoAvailable() const;
		void redo();
		//@}
		
		//! @name Editing
		/// These methods provide higher level editing functions for the
		/// script.
		////////////////////////////////////////////////////////////////////
		/// Copies the contents of this script to the clipboard in the
		/// application(). If specified then filter limits what is copied.
		void copy( ConstSetPtr filter=0 );
		/// Performs a copy() and then deletes the copied nodes.
		/// \undoable
		void cut( ConstSetPtr filter=0 );
		/// Pastes the contents of the global clipboard into the script.
		/// \undoable
		void paste();
		/// Removes Nodes from the script, making sure they are
		/// disconnected from the remaining Nodes and removed from the current
		/// selection. If specified then filter limits what is deleted. Note
		/// that it is also possible to call removeChild( node ) to remove
		/// nodes, and that the node will still be properly disconnected
		/// and unselected - this function is just a convenience method.
		/// \undoable
		void deleteNodes( ConstSetPtr filter=0 );
		//@}
			
		//! @name Script evaluation
		/// These methods allow the execution of python scripts in the
		/// context of the ScriptNode. The methods are only available on
		/// ScriptNode objects created from Python - they will throw Exceptions
		/// on nodes created from C++. This allows the ScriptNode class to be
		/// used in the C++ library without introducing dependencies on Python.
		/// Exceptions are also thrown if execution fails for any reason.
		////////////////////////////////////////////////////////////////////
		//@{
		typedef boost::signal<void ( ScriptNodePtr, const std::string )> ScriptExecutedSignal;
		typedef boost::signal<void ( ScriptNodePtr, const std::string, PyObject * )> ScriptEvaluatedSignal;
		/// Runs the specified python script.
		/// \todo I think we'll need a version of this that takes a python callable.
		/// We might expose that here or just introduce it in the binding layer for
		/// use from the python side only.
		virtual void execute( const std::string &pythonScript );
		/// This signal is emitted following successful execution of a script.
		ScriptExecutedSignal &scriptExecutedSignal();
		/// Evaluates the specified python expression. The caller owns a reference to
		/// the result, and must therefore decrement the reference count when
		/// appropriate.
		virtual PyObject *evaluate( const std::string &pythonExpression );
		/// This signal is emitted following sucessful evaluation of an expression. The PyObject *
		/// is the result of the script evaluation - slots must increment the reference count on
		/// this if they intend to keep the result.
		ScriptEvaluatedSignal &scriptEvaluatedSignal();
		//@}
		
		//! @name Serialisation
		/// Scripts may be serialised into a string form, which when executed
		/// in python will rebuild the node network.
		/// This process is used for both the saving and loading of scripts and
		/// for the cut and paste mechanism. As serialisation depends on
		/// python, these methods will throw Exceptions if called on ScriptNodes
		/// created from C++.
		/// \todo Very much need to consider how these will work for group
		/// nodes.
		////////////////////////////////////////////////////////////////////
		//@{
		/// Returns a string which when executed will recreate the child nodes
		/// of this script and the connections between them. A Set may be
		/// specified to limit the serialised nodes to those contained in the set.
		virtual std::string serialise( ConstSetPtr filter=0 ) const;
		/// Returns the plug which specifies the file used in all load and save
		/// operations.
		StringPlugPtr fileNamePlug();
		ConstStringPlugPtr fileNamePlug() const;
		/// Loads the script specified in the filename plug.
		virtual void load();
		/// Saves the script to the file specified by the filename plug.
		virtual void save() const;
		//@}

		//! @name Computation context
		/// This is a default context for computations to be performed in when
		/// no other context has been specified. There's no requirement to use it,
		/// and in fact when requesting output from Nodes any context may be used.
		/// The default context is typically used by the ui components.
		////////////////////////////////////////////////////////////////////
		//@{
		Context *context();
		const Context *context() const;
		//@}
		
		//! @name Frame range
		/// The ScriptNode defines the valid frame range using two numeric plugs.
		////////////////////////////////////////////////////////////////////
		//@{
		IntPlug *frameStartPlug();
		const IntPlug *frameStartPlug() const;
		IntPlug *frameEndPlug();
		const IntPlug *frameEndPlug() const;
		//@}
		
	private :
		
		bool selectionSetAcceptor( Set::ConstPtr s, Set::ConstMemberPtr m );
		StandardSetPtr m_selection;

		friend class Action;
		friend class UndoContext;
		
		typedef std::stack<UndoContext::State> UndoStateStack;
		typedef std::vector<ActionPtr> ActionVector;
		typedef boost::shared_ptr<ActionVector> ActionVectorPtr;
		typedef std::list<ActionVectorPtr> UndoList;
		typedef UndoList::iterator UndoIterator;
		
		UndoStateStack m_undoStateStack; // pushed and popped by the creation and destruction of UndoContexts
		ActionVectorPtr m_actionAccumulator; // Actions are accumulated here until the state stack hits 0 size
		UndoList m_undoList; // then the accumulated actions are transferred to this list for storage
		UndoIterator m_undoIterator; // points to the next thing to redo
			
		ScriptExecutedSignal m_scriptExecutedSignal;
		ScriptEvaluatedSignal m_scriptEvaluatedSignal;
	
		StringPlugPtr m_fileNamePlug;
		
		ContextPtr m_context;
		
		void childRemoved( GraphComponent *parent, GraphComponent *child );
		void plugSet( Plug *plug );
	
};

IE_CORE_DECLAREPTR( ScriptNode );

} // namespace Gaffer

#endif // GAFFER_SCRIPTNODE_H
