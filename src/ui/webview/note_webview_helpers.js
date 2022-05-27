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

// This function is necessary because the `:first-child` CSS selector does not account for text nodes.
function checkFirstChildNodeMatchesSelector(node, selector)
{
    let i;
    for (i = 0; i < node.childNodes.length; i++) {
        let child = node.childNodes[i];

        // Skip first child nodes that contain only whitespace.
        if(!(child.nodeType === Node.TEXT_NODE && child.textContent.trim().length === 0)) {
            break;
        }
    }

    let firstChild = node.childNodes[i];

    return (firstChild.nodeType !== Node.TEXT_NODE && firstChild.matches(selector))
}

function checkIfEmptyTextContent(node)
{
    let textContent = node.textContent;

    // Remove zero-width Unicode element
    textContent = textContent.replace('\u200B', '');

    return (textContent === null || textContent.trim() === "");
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

function getDescendantsAndLeaves(node, result)
{
    result = result || [];
    for (var i = 0; i < node.childNodes.length; i++) {
        result.push(node.childNodes[i])
        getDescendantsAndLeaves(node.childNodes[i], result);
    }
    return result;
}

function getFirstDescendantTextNode(element)
{
    nodeList = getDescendantsAndLeaves(element);
    for (var i = 0; i < nodeList.length; i++) {
        // Check for text node that is not blank
        if ((nodeList[i].nodeType === Node.TEXT_NODE && nodeList[i].nodeValue.trim().length != 0)) {
            return nodeList[i];
        }
    }

    return null;
}

function getSurroundingElementsWithFirstChildMatching(element, selector)
{
    let surroundingElements = [];

    let prevElement = element.previousElementSibling;
    while(prevElement && prevElement.querySelector(selector)) {
        if(!checkFirstChildNodeMatchesSelector(prevElement, selector)) {
            break;
        }

        surroundingElements.unshift(prevElement)
        prevElement = prevElement.previousElementSibling;
    }

    surroundingElements.push(element);

    let nextElement = element.nextElementSibling;
    while(nextElement && nextElement.querySelector(selector)) {
        if(!checkFirstChildNodeMatchesSelector(nextElement, selector)) {
            break;
        }

        surroundingElements.push(nextElement)
        nextElement = nextElement.nextElementSibling;
    }

    return surroundingElements;
}

function trimAllLineBreaks(string)
{
    if(string == null) {
        return;
    }

    return string.replace(/[\n\r]/g, "");
}

function wrapElementInTagName(element, tagName)
{
    wrapper = document.createElement(tagName);
    element.parentNode.insertBefore(wrapper, element);
    wrapper.appendChild(element);

    return wrapper;
}
