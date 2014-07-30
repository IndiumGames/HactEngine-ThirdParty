/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the documentation of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "OpenGLWindow.h"


/**
 * OpenGLWindow constructor.
 * 
 * @author Digia Plc
 * @since 2013-01-22
 */
OpenGLWindow::OpenGLWindow(QWindow* parent)
    : QWindow(parent),
      update_pending_(false),
      animating_(false),
      context_(NULL) {
    
    setSurfaceType(QWindow::OpenGLSurface);
}


/**
 * renderLater.
 * 
 * Posts an UpdateRequest to Qt, which will call renderNow() (via event()).
 * 
 * @author Digia Plc
 * @since 2013-01-22
 */
void OpenGLWindow::renderLater() {
    if (!update_pending_) {
        update_pending_ = true;
        QCoreApplication::postEvent(this, new QEvent(QEvent::UpdateRequest));
    }
}


/**
 * event.
 * 
 * Called when any event happens.
 * 
 * @return  True, if event was handled.
 * 
 * @author Digia Plc
 * @since 2013-01-22
 */
bool OpenGLWindow::event(QEvent* event) {
    if (event->type() == QEvent::UpdateRequest) {
        renderNow();
        return true;
    }
    else if (event->type() == QEvent::Close) {
        Quit();
        return true;
    }
    else if (event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease
             || event->type() == QEvent::MouseMove || event->type() == QEvent::MouseButtonDblClick
             || event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonRelease
             || event->type() == QEvent::Wheel) {
        
        DetectInput(event);
        return true;
    }
    else {
        return QWindow::event(event);
    }
}


/**
 * exposeEvent.
 * 
 * Called when the window is exposed?
 * 
 * @author Digia Plc
 * @since 2013-01-22
 */
void OpenGLWindow::exposeEvent(QExposeEvent* event) {
    Q_UNUSED(event);
    
    if (isExposed()) {
        renderNow();
    }
}


/**
 * resizeEvent.
 * 
 * Called when the window is resized?
 * 
 * @author Digia Plc
 * @since 2013-01-22
 */
void OpenGLWindow::resizeEvent(QResizeEvent* event) {
    Q_UNUSED(event);
    
    if (isExposed()) {
        renderNow();
    }
}


/**
 * renderNow.
 * 
 *  1. Creates an OpenGL context (if one doesn't exist)
 *  2. Initializes OpenGL 
 *  3. Initializes the GameEngine (calls GameEngine::Init() via initialize())
 *  4. Updates the GameEngine (calls GameEngine::Update() via render())
 *  5. Swaps buffers with OpenGL
 * 
 * @author Digia Plc
 * @since 2014-03-12
 */
void OpenGLWindow::renderNow() {
    if (!isExposed()) {
        return;
    }
    
    bool needsInitialize = false;
    update_pending_ = false;
    
    if (!context_) {
        // Create an OpenGL context
        context_ = new QOpenGLContext(this);
        context_->setFormat(requestedFormat());
        context_->create();
        
        needsInitialize = true;
    }
    
    // Make this the current window in the OpenGL context
    context_->makeCurrent(this);
    
    if (needsInitialize) {
        // Initialize OpenGL
        initializeOpenGLFunctions();
        
        // Initialize the GameEngine
        Init();
        
        // Make this the current window in the OpenGL context and swap buffers
        context_->makeCurrent(this);
        context_->swapBuffers(this);
    }
    
    // Update the GameEngine
    if (IsRunning()) {
        Update();
    }
    else {
        // Quit requested, shut down
        close();
        return;
    }
    
    // Make this the current window in the OpenGL context and swap buffers
    context_->makeCurrent(this);
    context_->swapBuffers(this);
    
    if (animating_) {
        renderLater();
    }
}


/**
 * setAnimating.
 * 
 * @author Digia Plc
 * @since 2013-01-22
 */
void OpenGLWindow::setAnimating(bool animating) {
    animating_ = animating;
    
    if (animating_) {
        renderLater();
    }
}

