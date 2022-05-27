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
function handleKeyDownEvent(window, content, event)
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

        // If selection is inside an element that has a checkbox, we want to insert and focus
        // a checkbox on a new line.
        if(checkFirstChildNodeMatchesSelector(selectionParent, "input.en-todo"))
        {
            handleReturnOnCheckbox(event, selection, selectionParent);
        }

        //
        // Emit signal notifying of new cursor position
        //

        // Doesn't work with empty newline, need to insert empty span
        // https://stackoverflow.com/questions/6846230/coordinates-of-selected-text-in-browser-page/6847328#6847328
        const selectionRange = selection.getRangeAt(0).cloneRange();
        selectionRange.collapse(true);

        // Insert temporary element so we can get the caret location
        let span = document.createElement("span");
        selectionRange.insertNode(span)
        const caretPosition = span.getClientRects()[0];
        span.parentNode.removeChild(span);

        parentWebEngine.domNewlineInserted(caretPosition.top);
    }
}


/*
 *  Handlers
 */
function handleReturnOnCheckbox(event, selection, selectionParent)
{
    // Do nothing if user does a "SHIFT+ENTER"
    if(event.shiftKey) {
        event.preventDefault();
        return;
    }

    const currentSelectionOffset = selection.getRangeAt(0).startOffset;
    let cursorPos = selection.anchorOffset;
    let textContent = selectionParent.textContent.trim();
    let contentsBeforeCursor = textContent.substring(0, cursorPos);
    let contentsAfterCursor = textContent.substring(cursorPos, textContent.length);

    let newCheckboxParent;

    // Copy the current checkbox div. This helps to preserve styles.
    let checkboxElement = selectionParent.innerHTML;

    if(selectionParent.parentNode.parentNode.tagName === 'UL')
    {
        // Traverse to the 'LI' parent node;
        selectionParent = selectionParent.parentNode;

        // If 'ENTER' pressed on empty checkbox element, split the parent 'UL' and insert a newline.
        if(checkIfEmptyTextContent(selectionParent))
        {
            // Insert new empty div between unordered lists
            let newEmptyDiv = document.createElement("DIV");
            newEmptyDiv.innerHTML = "<br>";
            let ulElement = selectionParent.parentNode;
            ulElement.parentNode.insertBefore(newEmptyDiv, ulElement.nextSibling);

            // Move following list elements to a new unordered list after the empty div
            let newUlElement = document.createElement("UL");
            while (x = selectionParent.nextElementSibling) newUlElement.appendChild(x);
            ulElement.parentNode.insertBefore(newUlElement, newEmptyDiv.nextSibling);

            selectionParent.remove();

            // Place cursor in the new empty div
            let newSelection = document.createRange();
            newSelection.setStart(newEmptyDiv, 0);
            selection.removeAllRanges();
            selection.addRange(newSelection);

            event.preventDefault();
            return;
        }
        // Otherwise insert a new checkbox list element
        else {
            // Use the same element tag to contain the new checkbox
            newCheckboxParent = document.createElement("LI");
            // When a child of 'LI' element, checkbox should be contained by a 'div'
            newCheckboxParent.innerHTML = "<div>" + checkboxElement + "</div>";
        }
    }
    // If checkbox is not inside an unordered-list, migrate the block of checkboxes to one
    else {
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

        event.preventDefault();
        return;
    }

    // Update text content of the selection row and new checkbox list element
    let selectionTextNode = getFirstDescendantTextNode(selectionParent)
    selectionTextNode.nodeValue = contentsBeforeCursor;
    //
    let newTextNode = getFirstDescendantTextNode(newCheckboxParent);
    newTextNode.nodeValue = contentsAfterCursor;

    let zeroWidthCharInserted = false;
    if(newTextNode.nodeValue == "") {
        // WebKit doesn't allow placing a caret in an empty span/text node.
        // Insert a zero-width Unicode char that gets selected on insertion (see: https://stackoverflow.com/a/14104166/3213602).
        newTextNode.nodeValue = '\u200B';
        zeroWidthCharInserted = true;
    }

    // Insert new checkbox row
    selectionParent.parentNode.insertBefore(newCheckboxParent, selectionParent.nextSibling);

    // Setup new checkbox default state + events
    let newCheckboxInput = newCheckboxParent.querySelector("input.en-todo");
    newCheckboxInput.removeAttribute('checked');
    checkboxAddEventListener(newCheckboxInput);

    // Create a new selection after the new checkbox
    let newSelection = document.createRange();
    newSelection.setStart(newTextNode, 0)
    // Select zero-width Unicode char if inserted
    if(zeroWidthCharInserted) {
        newSelection.setEnd(newTextNode, 1);
    }

    selection.removeAllRanges();
    selection.addRange(newSelection);

    event.preventDefault();
    return;
}
