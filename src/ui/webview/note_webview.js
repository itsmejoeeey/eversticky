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


var content;
var parentWebEngine;

window.onload = function() {
    main();
}

function domChanged() {
    // XMLSerializer needed to ensure string is valid XML
    // Otherwise, for example, .outerHTML returns <br> instead of <br/>
    parentWebEngine.domChanged(new XMLSerializer().serializeToString(content));
}

function checkIfOnlyChildOfNode(parentNode, childNode)
{
    let node = childNode;

    while(node !== parentNode) {
        node = node.parentNode;
        if(node.children.length > 1) {
            return false;
        }
    }
    return true;
}

function trimAllLineBreaks(string)
{
    if(string == null) {
        return;
    }

    return string.replace(/[\n\r]/g, "");
}

function main() {
    // Allow content of webview to be user-editable
    document.documentElement.contentEditable = true;


    new QWebChannel(qt.webChannelTransport, function (channel) {
        parentWebEngine = channel.objects.parentWebEngine;

        // Declare note observers
        var observer = new MutationObserver(function()
        {
            console.log('JS::DOM_CHANGED');
            domChanged();
        });
        //
        var resizeObserver = new ResizeObserver(function()
        {
            let body = document.body;
            let bodyStyle = window.getComputedStyle(body);
            let bodyMarginTop = parseInt(bodyStyle.getPropertyValue('margin-top'));
            let bodyMarginBottom = parseInt(bodyStyle.getPropertyValue('margin-bottom'));

            let newDomHeight = content.offsetHeight + bodyMarginTop + bodyMarginBottom;
            parentWebEngine.domHeightResized(newDomHeight);
        });


        // Handler for signal updateInnerHtml() from JsInterface
        parentWebEngine.updateInnerHtml.connect(function(text)
        {
            // Replace body with new note
            document.body.innerHTML = text;

            // Connect observers to new note container div
            content = document.getElementById('en-note');
            observer.observe(content, {attributes: true, characterData: true, childList: true, subtree: true});
            resizeObserver.observe(content);


            /*
             *  Usability enhancements
             */

            // Connect event listener to all checkboxes to ensure 'clicked' attribute
            // indicates the state of the checkbox.
            let checkboxes = document.querySelectorAll("input.en-todo");
            for(var i = 0; i < checkboxes.length; i++) {
                let checkbox = checkboxes[i];
                checkbox.addEventListener("click", function(event) {
                    if(event.currentTarget.checked) {
                        checkbox.removeAttribute('checked');
                    } else {
                        checkbox.setAttribute('checked', 'true');
                    }
                });
            };

            // Attached to window instead of #en-note to ensure we always
            // get the event (regardless of what's explicitly focussed).
            window.addEventListener("keydown", function(event)
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
                    if(selectionParent.querySelector(":scope > input[type=checkbox]"))
                    {
                        // Use the same element tag to contain the new checkbox
                        let newCheckboxParent = document.createElement(selectionParent.tagName);
                        newCheckboxParent.innerHTML = "<input type=\"checkbox\"><br clear=\"none\">";
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
            });


            // Disable note zoom
            window.addEventListener("wheel", function(event)
            {
                if(event.ctrlKey)
                    event.preventDefault();
            }, { passive: false });
        });


        // Notify JsInterface that initial DOM load is complete
        parentWebEngine.domLoaded();
    })
}
