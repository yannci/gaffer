##########################################################################
#  
#  Copyright (c) 2011-2012, John Haddon. All rights reserved.
#  
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are
#  met:
#  
#      * Redistributions of source code must retain the above
#        copyright notice, this list of conditions and the following
#        disclaimer.
#  
#      * Redistributions in binary form must reproduce the above
#        copyright notice, this list of conditions and the following
#        disclaimer in the documentation and/or other materials provided with
#        the distribution.
#  
#      * Neither the name of John Haddon nor the names of
#        any other contributors to this software may be used to endorse or
#        promote products derived from this software without specific prior
#        written permission.
#  
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
#  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
#  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
#  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
#  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
#  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
#  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
#  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
#  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
#  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
#  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#  
##########################################################################

from __future__ import with_statement

import IECore

import Gaffer
import GafferUI

def appendDefinitions( menuDefinition, prefix="" ) :

	menuDefinition.append( prefix + "/Undo", { "command" : undo, "shortCut" : "Ctrl+Z", "active" : __undoAvailable } )
	menuDefinition.append( prefix + "/Redo", { "command" : redo, "shortCut" : "Shift+Ctrl+Z", "active" : __redoAvailable } )
	menuDefinition.append( prefix + "/UndoDivider", { "divider" : True } )
	
	menuDefinition.append( prefix + "/Cut", { "command" : cut, "shortCut" : "Ctrl+X", "active" : __selectionAvailable } )
	menuDefinition.append( prefix + "/Copy", { "command" : copy, "shortCut" : "Ctrl+C", "active" : __selectionAvailable } )
	menuDefinition.append( prefix + "/Paste", { "command" : paste, "shortCut" : "Ctrl+V", "active" : __pasteAvailable } )
	menuDefinition.append( prefix + "/Delete", { "command" : delete, "shortCut" : "Backspace", "active" : __selectionAvailable } )
	menuDefinition.append( prefix + "/CutCopyPasteDeleteDivider", { "divider" : True } )

	menuDefinition.append( prefix + "/Select All", { "command" : selectAll, "shortCut" : "Ctrl+A" } )
	menuDefinition.append( prefix + "/Select None", { "command" : selectNone, "shortCut" : "Shift+Ctrl+A", "active" : __selectionAvailable } )

## A function suitable as the command for an Edit/Undo menu item. It must
# be invoked from a menu that has a ScriptWindow in its ancestry.
def undo( menu ) :

	scriptWindow = menu.ancestor( GafferUI.ScriptWindow )
	script = scriptWindow.scriptNode()
	if script.undoAvailable() :
		script.undo()
	
## A function suitable as the command for an Edit/Redo menu item. It must
# be invoked from a menu that has a ScriptWindow in its ancestry.
def redo( menu ) :

	scriptWindow = menu.ancestor( GafferUI.ScriptWindow )
	script = scriptWindow.scriptNode()
	if script.redoAvailable() :
		script.redo()

## A function suitable as the command for an Edit/Cut menu item. It must
# be invoked from a menu that has a ScriptWindow in its ancestry.
def cut( menu ) :

	scriptWindow = menu.ancestor( GafferUI.ScriptWindow )
	script = scriptWindow.scriptNode()

	with Gaffer.UndoContext( script ) :
		script.cut( script.selection() )

## A function suitable as the command for an Edit/Copy menu item. It must
# be invoked from a menu that has a ScriptWindow in its ancestry.
def copy( menu ) :

	scriptWindow = menu.ancestor( GafferUI.ScriptWindow )
	script = scriptWindow.scriptNode()
	script.copy( script.selection() )

## A function suitable as the command for an Edit/Paste menu item. It must
# be invoked from a menu that has a ScriptWindow in its ancestry.
def paste( menu ) :

	scriptWindow = menu.ancestor( GafferUI.ScriptWindow )
	script = scriptWindow.scriptNode()
	
	with Gaffer.UndoContext( script ) :
	
		script.paste()
	
## A function suitable as the command for an Edit/Delete menu item. It must
# be invoked from a menu that has a ScriptWindow in its ancestry.
def delete( menu ) :

	scriptWindow = menu.ancestor( GafferUI.ScriptWindow )
	script = scriptWindow.scriptNode()
	
	with Gaffer.UndoContext( script ) :
		
		script.deleteNodes( script.selection() )
	
## A function suitable as the command for an Edit/Select All menu item. It must
# be invoked from a menu that has a ScriptWindow in its ancestry.
def selectAll( menu ) :

	scriptWindow = menu.ancestor( GafferUI.ScriptWindow )
	script = scriptWindow.scriptNode()
	
	for c in script.children() :
		if c.isInstanceOf( Gaffer.Node.staticTypeId() ) :
			script.selection().add( c )	
			
## A function suitable as the command for an Edit/Select None menu item. It must
# be invoked from a menu that has a ScriptWindow in its ancestry.
def selectNone( menu ) :

	scriptWindow = menu.ancestor( GafferUI.ScriptWindow )
	script = scriptWindow.scriptNode()
	
	script.selection().clear()				

def __selectionAvailable( menu ) :

	scriptWindow = menu.ancestor( GafferUI.ScriptWindow )
	return True if scriptWindow.scriptNode().selection().size() else False
	
def __pasteAvailable( menu ) :

	scriptNode = menu.ancestor( GafferUI.ScriptWindow ).scriptNode()
	root = scriptNode.ancestor( Gaffer.ApplicationRoot.staticTypeId() )
	return isinstance( root.getClipboardContents(), IECore.StringData )

def __undoAvailable( menu ) :

	scriptNode = menu.ancestor( GafferUI.ScriptWindow ).scriptNode()
	return scriptNode.undoAvailable()
	
def __redoAvailable( menu ) :

	scriptNode = menu.ancestor( GafferUI.ScriptWindow ).scriptNode()
	return scriptNode.redoAvailable()