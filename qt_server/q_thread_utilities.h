#pragma once
#include <QObject>
#include <QApplication>
#include <functional>

// Qt 5 - port to main thread
static inline void post_to_main_thread(std::function<void()> fun)
{
  QObject signal_source;
  QObject::connect(&signal_source, &QObject::destroyed, qApp,
  [=](QObject*)
  {
        fun();
  },
  Qt::QueuedConnection);
}
