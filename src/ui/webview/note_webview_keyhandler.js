function handleKeyDownEvent(window, content, event)
{
    const selection = window.getSelection();

    if (event.key === "Backspace" || event.key === "Delete") {
        // If only a single empty div left in note when backspacing, prevent default
        // action from deleting the note parent node.
        if(content.children.length === 1 && // Quick check to prevent unnecessary traversal of the node tree
           trimAllLineBreaks(selection.focusNode.outerHTML) === "<div><br></div>" &&
           checkIfOnlyChildOfNode(content, selection.focusNode))
        {
            event.preventDefault();
        }
    }

    if (event.key === "Enter") {
        // Get the parent element at the start of the selection
        let selectionParent = selection.getRangeAt(0).startContainer.parentNode;

        // If selection is inside an element that has a checkbox, we want to insert and focus
        // a checkbox on a new line.
        if(selectionParent.querySelector(":scope > input.en-todo"))
        {
            // Use the same element tag to contain the new checkbox
            let newCheckboxParent = document.createElement(selectionParent.tagName);
            // Input element should match the one used in `NoteFormatter::convertFromEML()`
            newCheckboxParent.innerHTML = "<input class=\"en-todo\" src=\"data:image/gif;base64,R0lGODlhAQABAAD/ACwAAAAAAQABAAACADs=\" type=\"image\" /><br clear=\"none\">";
            selectionParent.parentNode.insertBefore(newCheckboxParent, selectionParent.nextSibling);

            // Create a new selection after the new checkbox
            let newSelection = document.createRange();
            newSelection.setStart(newCheckboxParent, 1);
            newSelection.collapse(true)

            selection.removeAllRanges();
            selection.addRange(newSelection);

            event.preventDefault();
        }

        // Doesn't work with empty newline, need to insert empty span
        // https://stackoverflow.com/questions/6846230/coordinates-of-selected-text-in-browser-page/6847328#6847328
        const selectionRange = window.getSelection().getRangeAt(0).cloneRange();
        selectionRange.collapse(true);

        // Insert temporary element so we can get the caret location
        let span = document.createElement("span");
        selectionRange.insertNode(span)
        const caretPosition = span.getClientRects()[0];
        span.parentNode.removeChild(span);

        parentWebEngine.domNewlineInserted(caretPosition.top);
    }
}
