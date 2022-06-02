/*
 * This file is part of the eversticky project (https://eversticky.joeeey.com).
 * Copyright (c) 2021 Joey Miller.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/*
 *  Main keyhandler
 */
function handleKeyDownEvent(event)
{
    const selection = window.getSelection();

    if (event.key === "Backspace" || event.key === "Delete")
    {
        // If only a single empty div left in note when backspacing, prevent default
        // action from deleting the note parent node.
        if(content.children.length === 1 && // Quick check to prevent unnecessary traversal of the node tree
           trimAllLineBreaks(selection.focusNode.outerHTML) === "<div><br></div>" &&
           checkIfOnlyChildOfNode(content, selection.focusNode))
        {
            event.preventDefault();
        }
    }

    if (event.key === "Enter")
    {
        // Get the parent element at the start of the selection
        let selectionParent = selection.getRangeAt(0).startContainer;

        // Traverse out of span or text nodes
        while(selectionParent.tagName === 'SPAN' || selectionParent.nodeType === Node.TEXT_NODE) {
            selectionParent = selectionParent.parentNode;
        }

        // If selection is inside an element that starts with a checkbox, we want to insert and focus
        // a checkbox on a new line.
        if(checkNestedFirstChildMatchesSelector(selectionParent, "input.en-todo"))
        {
            handleReturnOnCheckbox(event, selection, selectionParent);
        }
    }
}


/*
 *  Handlers
 */

function handleReturnOnCheckbox(event, selection, selectionParent)
{
    // Do default behaviour if user does a "SHIFT+ENTER"
    if(event.shiftKey) {
        return;
    }

    // If checkbox is not inside an unordered-list, migrate the block of checkboxes to one
    if(selectionParent.parentNode.parentNode.tagName !== 'UL')
    {
        const currentSelectionOffset = selection.getRangeAt(0).startOffset;

        let siblingElements = getSurroundingElementsWithFirstChildMatching(selectionParent, "input.en-todo");

        // Wrap the first checkbox row in an unordered-list, before adding the rest of the rows to the list.
        let newLiElement = wrapElementInTagName(siblingElements[0], "LI");
        let newUlElement = wrapElementInTagName(newLiElement, "UL");
        for(let i = 1; i < siblingElements.length; i++) {
            newUlElement.appendChild(
                wrapElementInTagName(siblingElements[i], "LI")
            );
        }

        // Restore the cursor position
        let newSelection = document.createRange();
        newSelection.setStart(getFirstDescendantTextNode(selectionParent), currentSelectionOffset);
        selection.removeAllRanges();
        selection.addRange(newSelection);
    }
    // If return pressed on empty checkbox element, insert a newline without the checkbox + unordered-list.
    else if(checkIfEmptyTextContent(selectionParent))
    {
        document.execCommand('delete', false); // delete checkbox element
        document.execCommand('insertUnorderedList', false); // toggle unordered list
    }
    // Otherwise just insert a checkbox on a newline.
    else
    {
        document.execCommand('insertParagraph', false);
        document.execCommand('insertHTML', false, '<input class=\"en-todo\" src=\"data:image/gif;base64,R0lGODlhAQABAAD/ACwAAAAAAQABAAACADs=\" type=\"image\" />');
    }

    event.preventDefault();
    return;
}
