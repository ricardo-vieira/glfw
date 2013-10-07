//========================================================================
// GLFW 3.0 - www.glfw.org
//------------------------------------------------------------------------
// Copyright (c) 2002-2006 Marcus Geelnard
// Copyright (c) 2006-2010 Camilla Berglund <elmindreda@elmindreda.org>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would
//    be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source
//    distribution.
//
//========================================================================

#include "internal.h"

// Internal key state used for sticky keys
#define _GLFW_STICK 3


// Sets the cursor mode for the specified window
//
static void setCursorMode(_GLFWwindow* window, int newMode)
{
    const int oldMode = window->cursorMode;

    if (newMode != GLFW_CURSOR_NORMAL &&
        newMode != GLFW_CURSOR_HIDDEN &&
        newMode != GLFW_CURSOR_DISABLED)
    {
        _glfwInputError(GLFW_INVALID_ENUM, NULL);
        return;
    }

    if (oldMode == newMode)
        return;

    window->cursorMode = newMode;

    if (window == _glfw.focusedWindow)
    {
        if (oldMode == GLFW_CURSOR_DISABLED)
        {
            window->cursorPosX = _glfw.cursorPosX;
            window->cursorPosY = _glfw.cursorPosY;

            _glfwPlatformSetCursorPos(window, _glfw.cursorPosX, _glfw.cursorPosY);
        }
        else if (newMode == GLFW_CURSOR_DISABLED)
        {
            int width, height;

            _glfw.cursorPosX = window->cursorPosX;
            _glfw.cursorPosY = window->cursorPosY;

            _glfwPlatformGetWindowSize(window, &width, &height);
            _glfwPlatformSetCursorPos(window, width / 2.0, height / 2.0);
        }

        _glfwPlatformSetCursorMode(window, newMode);
    }
}

// Set sticky keys mode for the specified window
//
static void setStickyKeys(_GLFWwindow* window, int enabled)
{
    if (window->stickyKeys == enabled)
        return;

    if (!enabled)
    {
        int i;

        // Release all sticky keys
        for (i = 0;  i <= GLFW_KEY_LAST;  i++)
        {
            if (window->key[i] == _GLFW_STICK)
                window->key[i] = GLFW_RELEASE;
        }
    }

    window->stickyKeys = enabled;
}

// Set sticky mouse buttons mode for the specified window
//
static void setStickyMouseButtons(_GLFWwindow* window, int enabled)
{
    if (window->stickyMouseButtons == enabled)
        return;

    if (!enabled)
    {
        int i;

        // Release all sticky mouse buttons
        for (i = 0;  i <= GLFW_MOUSE_BUTTON_LAST;  i++)
        {
            if (window->mouseButton[i] == _GLFW_STICK)
                window->mouseButton[i] = GLFW_RELEASE;
        }
    }

    window->stickyMouseButtons = enabled;
}


//////////////////////////////////////////////////////////////////////////
//////                         GLFW event API                       //////
//////////////////////////////////////////////////////////////////////////

void _glfwInputKey(_GLFWwindow* window, int key, int scancode, int action, int mods)
{
    GLboolean repeated = GL_FALSE;

    if (action == GLFW_RELEASE && window->key[key] == GLFW_RELEASE)
        return;

    if (key >= 0 && key <= GLFW_KEY_LAST)
    {
        if (action == GLFW_PRESS && window->key[key] == GLFW_PRESS)
            repeated = GL_TRUE;

        if (action == GLFW_RELEASE && window->stickyKeys)
            window->key[key] = _GLFW_STICK;
        else
            window->key[key] = (char) action;
    }

    if (repeated)
        action = GLFW_REPEAT;

    if (window->callbacks.key)
        window->callbacks.key((GLFWwindow*) window, key, scancode, action, mods);
}

void _glfwInputChar(_GLFWwindow* window, unsigned int codepoint)
{
    if (codepoint < 32 || (codepoint > 126 && codepoint < 160))
        return;

    if (window->callbacks.character)
        window->callbacks.character((GLFWwindow*) window, codepoint);
}

void _glfwInputScroll(_GLFWwindow* window, double xoffset, double yoffset)
{
    if (window->callbacks.scroll)
        window->callbacks.scroll((GLFWwindow*) window, xoffset, yoffset);
}

void _glfwInputMouseClick(_GLFWwindow* window, int button, int action, int mods)
{
    if (button < 0 || button > GLFW_MOUSE_BUTTON_LAST)
        return;

    // Register mouse button action
    if (action == GLFW_RELEASE && window->stickyMouseButtons)
        window->mouseButton[button] = _GLFW_STICK;
    else
        window->mouseButton[button] = (char) action;

    if (window->callbacks.mouseButton)
        window->callbacks.mouseButton((GLFWwindow*) window, button, action, mods);
}

void _glfwInputCursorMotion(_GLFWwindow* window, double x, double y)
{
    if (window->cursorMode == GLFW_CURSOR_DISABLED)
    {
        if (x == 0.0 && y == 0.0)
            return;

        window->cursorPosX += x;
        window->cursorPosY += y;
    }
    else
    {
        if (window->cursorPosX == x && window->cursorPosY == y)
            return;

        window->cursorPosX = x;
        window->cursorPosY = y;
    }

    if (window->callbacks.cursorPos)
    {
        window->callbacks.cursorPos((GLFWwindow*) window,
                                    window->cursorPosX,
                                    window->cursorPosY);
    }
}

void _glfwInputCursorEnter(_GLFWwindow* window, int entered)
{
    if (window->callbacks.cursorEnter)
        window->callbacks.cursorEnter((GLFWwindow*) window, entered);
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////

const char* _glfwGetKeyName(int key)
{
    switch (key)
    {
        // Printable keys
        case GLFW_KEY_A:            return "A";
        case GLFW_KEY_B:            return "B";
        case GLFW_KEY_C:            return "C";
        case GLFW_KEY_D:            return "D";
        case GLFW_KEY_E:            return "E";
        case GLFW_KEY_F:            return "F";
        case GLFW_KEY_G:            return "G";
        case GLFW_KEY_H:            return "H";
        case GLFW_KEY_I:            return "I";
        case GLFW_KEY_J:            return "J";
        case GLFW_KEY_K:            return "K";
        case GLFW_KEY_L:            return "L";
        case GLFW_KEY_M:            return "M";
        case GLFW_KEY_N:            return "N";
        case GLFW_KEY_O:            return "O";
        case GLFW_KEY_P:            return "P";
        case GLFW_KEY_Q:            return "Q";
        case GLFW_KEY_R:            return "R";
        case GLFW_KEY_S:            return "S";
        case GLFW_KEY_T:            return "T";
        case GLFW_KEY_U:            return "U";
        case GLFW_KEY_V:            return "V";
        case GLFW_KEY_W:            return "W";
        case GLFW_KEY_X:            return "X";
        case GLFW_KEY_Y:            return "Y";
        case GLFW_KEY_Z:            return "Z";
        case GLFW_KEY_1:            return "1";
        case GLFW_KEY_2:            return "2";
        case GLFW_KEY_3:            return "3";
        case GLFW_KEY_4:            return "4";
        case GLFW_KEY_5:            return "5";
        case GLFW_KEY_6:            return "6";
        case GLFW_KEY_7:            return "7";
        case GLFW_KEY_8:            return "8";
        case GLFW_KEY_9:            return "9";
        case GLFW_KEY_0:            return "0";
        case GLFW_KEY_SPACE:        return "SPACE";
        case GLFW_KEY_MINUS:        return "MINUS";
        case GLFW_KEY_EQUAL:        return "EQUAL";
        case GLFW_KEY_LEFT_BRACKET: return "LEFT BRACKET";
        case GLFW_KEY_RIGHT_BRACKET: return "RIGHT BRACKET";
        case GLFW_KEY_BACKSLASH:    return "BACKSLASH";
        case GLFW_KEY_SEMICOLON:    return "SEMICOLON";
        case GLFW_KEY_APOSTROPHE:   return "APOSTROPHE";
        case GLFW_KEY_GRAVE_ACCENT: return "GRAVE ACCENT";
        case GLFW_KEY_COMMA:        return "COMMA";
        case GLFW_KEY_PERIOD:       return "PERIOD";
        case GLFW_KEY_SLASH:        return "SLASH";
        case GLFW_KEY_WORLD_1:      return "WORLD 1";
        case GLFW_KEY_WORLD_2:      return "WORLD 2";
        case GLFW_KEY_WORLD_3:      return "WORLD 3";

        // Function keys
        case GLFW_KEY_ESCAPE:       return "ESCAPE";
        case GLFW_KEY_F1:           return "F1";
        case GLFW_KEY_F2:           return "F2";
        case GLFW_KEY_F3:           return "F3";
        case GLFW_KEY_F4:           return "F4";
        case GLFW_KEY_F5:           return "F5";
        case GLFW_KEY_F6:           return "F6";
        case GLFW_KEY_F7:           return "F7";
        case GLFW_KEY_F8:           return "F8";
        case GLFW_KEY_F9:           return "F9";
        case GLFW_KEY_F10:          return "F10";
        case GLFW_KEY_F11:          return "F11";
        case GLFW_KEY_F12:          return "F12";
        case GLFW_KEY_F13:          return "F13";
        case GLFW_KEY_F14:          return "F14";
        case GLFW_KEY_F15:          return "F15";
        case GLFW_KEY_F16:          return "F16";
        case GLFW_KEY_F17:          return "F17";
        case GLFW_KEY_F18:          return "F18";
        case GLFW_KEY_F19:          return "F19";
        case GLFW_KEY_F20:          return "F20";
        case GLFW_KEY_F21:          return "F21";
        case GLFW_KEY_F22:          return "F22";
        case GLFW_KEY_F23:          return "F23";
        case GLFW_KEY_F24:          return "F24";
        case GLFW_KEY_F25:          return "F25";
        case GLFW_KEY_UP:           return "UP";
        case GLFW_KEY_DOWN:         return "DOWN";
        case GLFW_KEY_LEFT:         return "LEFT";
        case GLFW_KEY_RIGHT:        return "RIGHT";
        case GLFW_KEY_LEFT_SHIFT:   return "LEFT SHIFT";
        case GLFW_KEY_RIGHT_SHIFT:  return "RIGHT SHIFT";
        case GLFW_KEY_LEFT_CONTROL: return "LEFT CONTROL";
        case GLFW_KEY_RIGHT_CONTROL: return "RIGHT CONTROL";
        case GLFW_KEY_LEFT_ALT:     return "LEFT ALT";
        case GLFW_KEY_RIGHT_ALT:    return "RIGHT ALT";
        case GLFW_KEY_TAB:          return "TAB";
        case GLFW_KEY_ENTER:        return "ENTER";
        case GLFW_KEY_BACKSPACE:    return "BACKSPACE";
        case GLFW_KEY_INSERT:       return "INSERT";
        case GLFW_KEY_DELETE:       return "DELETE";
        case GLFW_KEY_PAGE_UP:      return "PAGE UP";
        case GLFW_KEY_PAGE_DOWN:    return "PAGE DOWN";
        case GLFW_KEY_HOME:         return "HOME";
        case GLFW_KEY_END:          return "END";
        case GLFW_KEY_KP_0:         return "KEYPAD 0";
        case GLFW_KEY_KP_1:         return "KEYPAD 1";
        case GLFW_KEY_KP_2:         return "KEYPAD 2";
        case GLFW_KEY_KP_3:         return "KEYPAD 3";
        case GLFW_KEY_KP_4:         return "KEYPAD 4";
        case GLFW_KEY_KP_5:         return "KEYPAD 5";
        case GLFW_KEY_KP_6:         return "KEYPAD 6";
        case GLFW_KEY_KP_7:         return "KEYPAD 7";
        case GLFW_KEY_KP_8:         return "KEYPAD 8";
        case GLFW_KEY_KP_9:         return "KEYPAD 9";
        case GLFW_KEY_KP_DIVIDE:    return "KEYPAD DIVIDE";
        case GLFW_KEY_KP_MULTIPLY:  return "KEYPAD MULTPLY";
        case GLFW_KEY_KP_SUBTRACT:  return "KEYPAD SUBTRACT";
        case GLFW_KEY_KP_ADD:       return "KEYPAD ADD";
        case GLFW_KEY_KP_DECIMAL:   return "KEYPAD DECIMAL";
        case GLFW_KEY_KP_EQUAL:     return "KEYPAD EQUAL";
        case GLFW_KEY_KP_ENTER:     return "KEYPAD ENTER";
        case GLFW_KEY_PRINT_SCREEN: return "PRINT SCREEN";
        case GLFW_KEY_NUM_LOCK:     return "NUM LOCK";
        case GLFW_KEY_CAPS_LOCK:    return "CAPS LOCK";
        case GLFW_KEY_SCROLL_LOCK:  return "SCROLL LOCK";
        case GLFW_KEY_PAUSE:        return "PAUSE";
        case GLFW_KEY_LEFT_SUPER:   return "LEFT SUPER";
        case GLFW_KEY_RIGHT_SUPER:  return "RIGHT SUPER";
        case GLFW_KEY_MENU:         return "MENU";
        case GLFW_KEY_UNKNOWN:      return "UNKNOWN";
    }

    return NULL;
}


//////////////////////////////////////////////////////////////////////////
//////                        GLFW public API                       //////
//////////////////////////////////////////////////////////////////////////

GLFWAPI int glfwGetInputMode(GLFWwindow* handle, int mode)
{
    _GLFWwindow* window = (_GLFWwindow*) handle;

    _GLFW_REQUIRE_INIT_OR_RETURN(0);

    switch (mode)
    {
        case GLFW_CURSOR:
            return window->cursorMode;
        case GLFW_STICKY_KEYS:
            return window->stickyKeys;
        case GLFW_STICKY_MOUSE_BUTTONS:
            return window->stickyMouseButtons;
        default:
            _glfwInputError(GLFW_INVALID_ENUM, NULL);
            return 0;
    }
}

GLFWAPI void glfwSetInputMode(GLFWwindow* handle, int mode, int value)
{
    _GLFWwindow* window = (_GLFWwindow*) handle;

    _GLFW_REQUIRE_INIT();

    switch (mode)
    {
        case GLFW_CURSOR:
            setCursorMode(window, value);
            break;
        case GLFW_STICKY_KEYS:
            setStickyKeys(window, value ? GL_TRUE : GL_FALSE);
            break;
        case GLFW_STICKY_MOUSE_BUTTONS:
            setStickyMouseButtons(window, value ? GL_TRUE : GL_FALSE);
            break;
        default:
            _glfwInputError(GLFW_INVALID_ENUM, NULL);
            break;
    }
}

GLFWAPI const char* glfwGetKeyName(int key)
{
    _GLFW_REQUIRE_INIT_OR_RETURN(NULL);

    if (key < 0 || key > GLFW_KEY_LAST)
    {
        _glfwInputError(GLFW_INVALID_ENUM, "The specified key is invalid");
        return NULL;
    }

    return _glfwPlatformGetKeyName(key);
}


GLFWAPI int glfwGetKey(GLFWwindow* handle, int key)
{
    _GLFWwindow* window = (_GLFWwindow*) handle;

    _GLFW_REQUIRE_INIT_OR_RETURN(GLFW_RELEASE);

    if (key < 0 || key > GLFW_KEY_LAST)
    {
        _glfwInputError(GLFW_INVALID_ENUM, "The specified key is invalid");
        return GLFW_RELEASE;
    }

    if (window->key[key] == _GLFW_STICK)
    {
        // Sticky mode: release key now
        window->key[key] = GLFW_RELEASE;
        return GLFW_PRESS;
    }

    return (int) window->key[key];
}

GLFWAPI int glfwGetMouseButton(GLFWwindow* handle, int button)
{
    _GLFWwindow* window = (_GLFWwindow*) handle;

    _GLFW_REQUIRE_INIT_OR_RETURN(GLFW_RELEASE);

    if (button < 0 || button > GLFW_MOUSE_BUTTON_LAST)
    {
        _glfwInputError(GLFW_INVALID_ENUM,
                        "The specified mouse button is invalid");
        return GLFW_RELEASE;
    }

    if (window->mouseButton[button] == _GLFW_STICK)
    {
        // Sticky mode: release mouse button now
        window->mouseButton[button] = GLFW_RELEASE;
        return GLFW_PRESS;
    }

    return (int) window->mouseButton[button];
}

GLFWAPI void glfwGetCursorPos(GLFWwindow* handle, double* xpos, double* ypos)
{
    _GLFWwindow* window = (_GLFWwindow*) handle;

    _GLFW_REQUIRE_INIT();

    if (xpos)
        *xpos = window->cursorPosX;

    if (ypos)
        *ypos = window->cursorPosY;
}

GLFWAPI void glfwSetCursorPos(GLFWwindow* handle, double xpos, double ypos)
{
    _GLFWwindow* window = (_GLFWwindow*) handle;

    _GLFW_REQUIRE_INIT();

    if (_glfw.focusedWindow != window)
        return;

    // Don't do anything if the cursor position did not change
    if (xpos == window->cursorPosX && ypos == window->cursorPosY)
        return;

    // Set GLFW cursor position
    window->cursorPosX = xpos;
    window->cursorPosY = ypos;

    // Do not move physical cursor if it is disabled
    if (window->cursorMode == GLFW_CURSOR_DISABLED)
        return;

    // Update physical cursor position
    _glfwPlatformSetCursorPos(window, xpos, ypos);
}

GLFWAPI GLFWkeyfun glfwSetKeyCallback(GLFWwindow* handle, GLFWkeyfun cbfun)
{
    _GLFWwindow* window = (_GLFWwindow*) handle;
    _GLFW_REQUIRE_INIT_OR_RETURN(NULL);
    _GLFW_SWAP_POINTERS(window->callbacks.key, cbfun);
    return cbfun;
}

GLFWAPI GLFWcharfun glfwSetCharCallback(GLFWwindow* handle, GLFWcharfun cbfun)
{
    _GLFWwindow* window = (_GLFWwindow*) handle;
    _GLFW_REQUIRE_INIT_OR_RETURN(NULL);
    _GLFW_SWAP_POINTERS(window->callbacks.character, cbfun);
    return cbfun;
}

GLFWAPI GLFWmousebuttonfun glfwSetMouseButtonCallback(GLFWwindow* handle,
                                                      GLFWmousebuttonfun cbfun)
{
    _GLFWwindow* window = (_GLFWwindow*) handle;
    _GLFW_REQUIRE_INIT_OR_RETURN(NULL);
    _GLFW_SWAP_POINTERS(window->callbacks.mouseButton, cbfun);
    return cbfun;
}

GLFWAPI GLFWcursorposfun glfwSetCursorPosCallback(GLFWwindow* handle,
                                                  GLFWcursorposfun cbfun)
{
    _GLFWwindow* window = (_GLFWwindow*) handle;
    _GLFW_REQUIRE_INIT_OR_RETURN(NULL);
    _GLFW_SWAP_POINTERS(window->callbacks.cursorPos, cbfun);
    return cbfun;
}

GLFWAPI GLFWcursorenterfun glfwSetCursorEnterCallback(GLFWwindow* handle,
                                                      GLFWcursorenterfun cbfun)
{
    _GLFWwindow* window = (_GLFWwindow*) handle;
    _GLFW_REQUIRE_INIT_OR_RETURN(NULL);
    _GLFW_SWAP_POINTERS(window->callbacks.cursorEnter, cbfun);
    return cbfun;
}

GLFWAPI GLFWscrollfun glfwSetScrollCallback(GLFWwindow* handle,
                                            GLFWscrollfun cbfun)
{
    _GLFWwindow* window = (_GLFWwindow*) handle;
    _GLFW_REQUIRE_INIT_OR_RETURN(NULL);
    _GLFW_SWAP_POINTERS(window->callbacks.scroll, cbfun);
    return cbfun;
}

