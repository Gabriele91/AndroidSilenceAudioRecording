#pragma once
#include <QFrame>
#include <QMouseEvent>
#include <functional>

class q_frame_double_click : public QFrame
{

    Q_OBJECT

public:

    explicit q_frame_double_click(QWidget *parent = 0)
    :QFrame(parent)
    {
    }

    void set_callback(std::function< void(bool) > callback)
    {
        m_callback = callback;
    }

protected:

    void mouseDoubleClickEvent(QMouseEvent *event)  Q_DECL_OVERRIDE
    {
        //call
        if(m_callback) m_callback(m_first);
        //update
        m_first = !m_first;
    }

    std::function< void(bool) >     m_callback{ nullptr };
    bool                            m_first   {  true   };
};
