#pragma once

#include <Irrlicht.h>
 
namespace Gwen
{
    namespace Input
    {
        class Irrlicht
        {
        private:
            Gwen::Controls::Canvas* m_Canvas;
            irr::s32 MouseX;
            irr::s32 MouseY;
 
        public:
            Irrlicht(Gwen::Controls::Canvas* Canvas) :
                m_Canvas(Canvas)
            {
                MouseX = 0;
                MouseY = 0;
            }
 
            ~Irrlicht()
            {}
 
            bool HandleEvents(const irr::SEvent& IrrEvent)
            {
                switch (IrrEvent.EventType)
                {
                    case irr::EET_MOUSE_INPUT_EVENT:
                    {
                        switch (IrrEvent.MouseInput.Event)
                        {
                            case irr::EMIE_MOUSE_MOVED:
                            {
                                int x = IrrEvent.MouseInput.X;
                                int y = IrrEvent.MouseInput.Y;
                                int dx = x - MouseX;
                                int dy = y - MouseY;
                                MouseX = x;
                                MouseY = y;
                                return m_Canvas->InputMouseMoved(x, y, dx, dy);
                            }
 
                            case irr::EMIE_MOUSE_WHEEL:
                            {
                                return m_Canvas->InputMouseWheel(IrrEvent.MouseInput.Wheel);
                            }
 
                            case irr::EMIE_LMOUSE_PRESSED_DOWN:
                            {
                                return m_Canvas->InputMouseButton(0, true);
                            }
 
                            case irr::EMIE_LMOUSE_LEFT_UP:
                            {
                                return m_Canvas->InputMouseButton(0, false);
                            }
 
                            case irr::EMIE_RMOUSE_PRESSED_DOWN:
                            {
                                return m_Canvas->InputMouseButton(1, true);
                            }
 
                            case irr::EMIE_RMOUSE_LEFT_UP:
                            {
                                return m_Canvas->InputMouseButton(1, false);
                            }
 
                            case irr::EMIE_MMOUSE_PRESSED_DOWN:
                            {
                                return m_Canvas->InputMouseButton(2, true);
                            }
 
                            case irr::EMIE_MMOUSE_LEFT_UP:
                            {
                                return m_Canvas->InputMouseButton(2, false);
                            }
 
                            default:
                            {
                                return false;
                            }
                        }
                    }
 
                    case irr::EET_KEY_INPUT_EVENT:
                    {
                        switch (IrrEvent.KeyInput.Key)
                        {
                            case irr::KEY_SHIFT:
                            {
                                return m_Canvas->InputKey(Gwen::Key::Shift, IrrEvent.KeyInput.PressedDown);
                            }
 
                            case irr::KEY_RETURN:
                            {
                                return m_Canvas->InputKey(Gwen::Key::Return, IrrEvent.KeyInput.PressedDown);
                            }
 
                            case irr::KEY_BACK:
                            {
                                return m_Canvas->InputKey(Gwen::Key::Backspace, IrrEvent.KeyInput.PressedDown);
                            }
 
                            case irr::KEY_DELETE:
                            {
                                return m_Canvas->InputKey(Gwen::Key::Delete, IrrEvent.KeyInput.PressedDown);
                            }
 
                            case irr::KEY_LEFT:
                            {
                                return m_Canvas->InputKey(Gwen::Key::Left, IrrEvent.KeyInput.PressedDown);
                            }
 
                            case irr::KEY_RIGHT:
                            {
                                return m_Canvas->InputKey(Gwen::Key::Right, IrrEvent.KeyInput.PressedDown);
                            }
 
                            case irr::KEY_TAB:
                            {
                                return m_Canvas->InputKey(Gwen::Key::Tab, IrrEvent.KeyInput.PressedDown);
                            }
 
                            case irr::KEY_SPACE:
                            {
                                return m_Canvas->InputKey(Gwen::Key::Space, IrrEvent.KeyInput.PressedDown);
                            }
 
                            case irr::KEY_HOME:
                            {
                                return m_Canvas->InputKey(Gwen::Key::Home, IrrEvent.KeyInput.PressedDown);
                            }
 
                            case irr::KEY_END:
                            {
                                return m_Canvas->InputKey(Gwen::Key::End, IrrEvent.KeyInput.PressedDown);
                            }
 
                            case irr::KEY_CONTROL:
                            {
                                return m_Canvas->InputKey(Gwen::Key::Control, IrrEvent.KeyInput.PressedDown);
                            }
 
                            case irr::KEY_UP:
                            {
                                return m_Canvas->InputKey(Gwen::Key::Up, IrrEvent.KeyInput.PressedDown);
                            }
 
                            case irr::KEY_DOWN:
                            {
                                return m_Canvas->InputKey(Gwen::Key::Down, IrrEvent.KeyInput.PressedDown);
                            }
 
                            default:
                            {
                                return m_Canvas->InputCharacter((Gwen::UnicodeChar)IrrEvent.KeyInput.Char);
                            }
                        }
                    }
 
                    default:
                    {
                        return false;
                    }
                }
            }
        };
    }
}