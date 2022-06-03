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

var prevCaretY;
var prevCaretYStart;
var prevCaretYEnd;

window.onload = function() {
    main();
}

function main()
{
    // Allow content of webview to be user-editable
    document.documentElement.contentEditable = true;

    const webChannel = new QWebChannel(qt.webChannelTransport, function (channel) {
        parentWebEngine = channel.objects.parentWebEngine;

        // Declare note observers
        var observer = new MutationObserver(function()
        {
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

            // Watch for selection changes
            document.addEventListener("selectionchange", selectionChangeEvent);


            /*
             *  Usability enhancements
             */

            // Connect event listener to all checkboxes to ensure 'clicked' attribute
            // indicates the state of the checkbox.
            let checkboxes = document.querySelectorAll("input.en-todo");
            for(var i = 0; i < checkboxes.length; i++) {
                let checkbox = checkboxes[i];
                checkboxAddEventListener(checkbox);
            };

            // Attached to window instead of #en-note to ensure we always
            // get the event (regardless of what's explicitly focussed).
            window.addEventListener("keydown", handleKeyDownEvent);

            // Disable note zoom
            window.addEventListener("wheel", wheelEvent, { passive: false });
        });


        parentWebEngine.insertCheckbox.connect(function(){
            insertCheckbox();
        });
        parentWebEngine.insertDivider.connect(function(){
            insertDivider();
        });
        parentWebEngine.insertBulletedList.connect(function(){
            insertBulletedList();
        });
        parentWebEngine.insertNumberedList.connect(function(){
            insertNumberedList();
        });


        // Notify JsInterface that initial DOM load is complete
        parentWebEngine.domLoaded();
    })
}


/*
 *
 */
function checkboxAddEventListener(checkbox)
{
    checkbox.addEventListener("click", checkboxClickEvent);

    // Don't allow right-clicking on checkboxes
    checkbox.addEventListener("contextmenu", checkboxContextMenuEvent);
}

function checkboxClickEvent(event)
{
    let checkbox = event.currentTarget;
    if(checkbox.checked) {
        checkbox.removeAttribute('checked');
    } else {
        checkbox.setAttribute('checked', 'true');
    }
}

function checkboxContextMenuEvent(event)
{
    event.preventDefault();
    return;
}

function checkCaretYPosChanged(selection)
{
    const selectionRangeStart = selection.getRangeAt(0).cloneRange();
    selectionRangeStart.collapse(true);
    const caretYStart = getCaretPositionAtSelection(selectionRangeStart).top;

    const selectionRangeEnd = selection.getRangeAt(0).cloneRange();
    selectionRangeEnd.collapse(false);
    const caretYEnd = getCaretPositionAtSelection(selectionRangeEnd).top;

    if(caretYStart !== prevCaretYStart) {
        parentWebEngine.domCaretYPosChanged(caretYStart);
    } else if(caretYEnd !== prevCaretYEnd) {
        parentWebEngine.domCaretYPosChanged(caretYEnd);
    }

    prevCaretYStart = caretYStart;
    prevCaretYEnd = caretYEnd;
}

function domChanged()
{
    // XMLSerializer needed to ensure string is valid XML
    // Otherwise, for example, .outerHTML returns <br> instead of <br/>
    parentWebEngine.domChanged(new XMLSerializer().serializeToString(content));
}

function insertCheckbox()
{
    document.execCommand('insertHTML', false, '<input class=\"en-todo\" src=\"data:image/gif;base64,R0lGODlhAQABAAD/ACwAAAAAAQABAAACADs=\" type=\"image\" />');
}

function insertDivider()
{
    document.execCommand('insertHorizontalRule', false);
}

function insertBulletedList()
{
    document.execCommand('insertUnorderedList', false);
}

function insertNumberedList()
{
    document.execCommand('insertOrderedList', false);
}

function selectionChangeEvent(event)
{
    const selection = window.getSelection();
    checkCaretYPosChanged(selection);
}

function wheelEvent(event)
{
    if(event.ctrlKey)
        event.preventDefault();
        return;
}
