#include <QMouseEvent>
#include <QGuiApplication>

#include "NGLScene.h"
#include <ngl/Transformation.h>
#include <ngl/NGLInit.h>
#include <ngl/VAOFactory.h>
#include <ngl/SimpleVAO.h>
#include <ngl/ShaderLib.h>
#include <ngl/Vec2.h>
#include <ngl/Random.h>
#include <chrono>
#include <fmt/format.h>
#include <iostream>

NGLScene::NGLScene(uint32_t _w, uint32_t _h, uint32_t _numParticles)
{
  setTitle("Particle Update Using Compute Shaders");
  m_gridWidth = _w;
  m_gridHeight = _h;
  m_numParticles = _numParticles;
}

NGLScene::~NGLScene()
{
  std::cout << "Shutting down NGL, removing VAO's and Shaders\n";
}

void NGLScene::resizeGL(int _w, int _h)
{
  m_project = ngl::perspective(65.0f, static_cast<float>(_w) / _h, 0.05f, 350.0f);
  m_win.width = static_cast<int>(_w * devicePixelRatio());
  m_win.height = static_cast<int>(_h * devicePixelRatio());
  m_text->setScreenSize(_w, _h);
}

constexpr auto ComputeShader = "ComputeShader";
void NGLScene::initializeGL()
{
  // we need to initialise the NGL lib which will load all of the OpenGL functions, this must
  // be done once we have a valid GL context but before we call any GL commands. If we dont do
  // this everything will crash
  ngl::NGLInit::initialize();

  glClearColor(0.4f, 0.4f, 0.4f, 1.0f); // Grey Background
  // enable depth testing for drawing

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_MULTISAMPLE);
  // Now we will create a basic Camera from the graphics library
  // This is a static camera so it only needs to be set once
  // First create Values for the camera position
  float offset = (m_gridWidth > m_gridHeight) ? m_gridWidth : m_gridHeight;
  ngl::Vec3 from(0, offset + 2, 0);
  ngl::Vec3 to(0, 0, 0);
  ngl::Vec3 up(0, 0, 1);

  m_view = ngl::lookAt(from, to, up);
  // set the shape using FOV 45 Aspect Ratio based on Width and Height
  // The final two are near and far clipping planes of 0.5 and 10
  m_project = ngl::perspective(65, 1024.0f / 720.0f, 0.01f, 150);

  // now to load the shader and set the values
  // grab an instance of shader manager
  ngl::ShaderLib::loadShader("PosDir", "shaders/PosDirVertex.glsl", "shaders/PosDirFragment.glsl", "shaders/PosDirGeo.glsl");
  ngl::ShaderLib::use("PosDir");
  glViewport(0, 0, width(), height());
  ngl::VAOPrimitives::createLineGrid("lineGrid", m_gridWidth, m_gridHeight, 2);
  ngl::ShaderLib::use(ngl::nglColourShader);
  ngl::ShaderLib::setUniform("Colour", 1.0f, 1.0f, 1.0f, 1.0f);
  m_text = std::make_unique<ngl::Text>("fonts/Arial.ttf", 18);
  m_text->setColour(1.0f, 1.0f, 0.0f);

  // load the Compute Shader

  ngl::ShaderLib::createShaderProgram(ComputeShader);
  ngl::ShaderLib::attachShader("Compute", ngl::ShaderType::COMPUTE);
  ngl::ShaderLib::loadShaderSource("Compute", "shaders/UpdateParticlesCompute.glsl");
  ngl::ShaderLib::compileShader("Compute");
  ngl::ShaderLib::attachShaderToProgram(ComputeShader, "Compute");
  ngl::ShaderLib::linkProgramObject(ComputeShader);
  ngl::ShaderLib::use(ComputeShader);
  ngl::ShaderLib::setUniform("xsize", m_gridWidth / 2.0f);
  ngl::ShaderLib::setUniform("zsize", m_gridHeight / 2.0f);
  startTimer(20);

  glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &m_group);
  m_grid = std::make_unique<Grid>(m_gridWidth, m_gridHeight, m_numParticles, m_group);
  std::cout << "Work group size " << m_group << '\n';
}

void NGLScene::paintGL()
{
  // clear the screen and depth buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  // Rotation based on the mouse position for our global transform
  ngl::Transformation trans;
  // Rotation based on the mouse position for our global transform
  auto rotX = ngl::Mat4::rotateX(m_win.spinXFace);
  auto rotY = ngl::Mat4::rotateY(m_win.spinYFace);
  // multiply the rotations
  m_mouseGlobalTX = rotX * rotY;
  // add the translations
  //  m_mouseGlobalTX.m_m[3][0] = m_modelPos.m_x;
  m_mouseGlobalTX.m_m[3][1] = m_modelPos.m_z;
  //  m_mouseGlobalTX.m_m[3][2] = m_modelPos.m_z;
  ngl::ShaderLib::use("PosDir");
  ngl::Mat4 MVP;
  MVP = m_project * m_view * m_mouseGlobalTX;

  ngl::ShaderLib::setUniform("MVP", MVP);
  glPointSize(20);
  auto drawbegin = std::chrono::steady_clock::now();

  m_grid->draw();
  auto drawend = std::chrono::steady_clock::now();

  std::string text = fmt::format("Draw took {0} uS", std::chrono::duration_cast<std::chrono::microseconds>(drawend - drawbegin).count());
  m_text->renderText(10, 680, text);
  text = fmt::format("Update took {0} uS for {1} particles", m_updateTime, m_grid->getNumParticles());
  m_text->renderText(10, 660, text);

  ngl::ShaderLib::use(ngl::nglColourShader);
  ngl::ShaderLib::setUniform("MVP", MVP);
  ngl::VAOPrimitives::draw("lineGrid");
}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::mouseMoveEvent(QMouseEvent *_event)
{
// note the method buttons() is the button state when event was called
// that is different from button() which is used to check which button was
// pressed when the mousePress/Release event is generated
#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
  auto position = _event->position();
#else
  auto position = _event->pos();
#endif
  if (m_win.rotate && _event->buttons() == Qt::LeftButton)
  {
    int diffx = position.x() - m_win.origX;
    int diffy = position.y() - m_win.origY;
    m_win.spinXFace += static_cast<int>(0.5f * diffy);
    m_win.spinYFace += static_cast<int>(0.5f * diffx);
    m_win.origX = position.x();
    m_win.origY = position.y();
    update();
  }
  // right mouse translate code
  else if (m_win.translate && _event->buttons() == Qt::RightButton)
  {
    int diffX = static_cast<int>(position.x() - m_win.origXPos);
    int diffY = static_cast<int>(position.y() - m_win.origYPos);
    m_win.origXPos = position.x();
    m_win.origYPos = position.y();
    m_modelPos.m_x += INCREMENT * diffX;
    m_modelPos.m_y -= INCREMENT * diffY;
    update();
  }
}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::mousePressEvent(QMouseEvent *_event)
{
// that method is called when the mouse button is pressed in this case we
// store the value where the maouse was clicked (x,y) and set the Rotate flag to true
#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
  auto position = _event->position();
#else
  auto position = _event->pos();
#endif
  if (_event->button() == Qt::LeftButton)
  {
    m_win.origX = position.x();
    m_win.origY = position.y();
    m_win.rotate = true;
  }
  // right mouse translate mode
  else if (_event->button() == Qt::RightButton)
  {
    m_win.origXPos = position.x();
    m_win.origYPos = position.y();
    m_win.translate = true;
  }
}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::mouseReleaseEvent(QMouseEvent *_event)
{
  // that event is called when the mouse button is released
  // we then set Rotate to false
  if (_event->button() == Qt::LeftButton)
  {
    m_win.rotate = false;
  }
  // right mouse translate mode
  if (_event->button() == Qt::RightButton)
  {
    m_win.translate = false;
  }
}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::wheelEvent(QWheelEvent *_event)
{

  // check the diff of the wheel position (0 means no change)
  if (_event->angleDelta().y() > 0)
  {
    m_modelPos.m_z += ZOOM;
  }
  else if (_event->angleDelta().y() < 0)
  {
    m_modelPos.m_z -= ZOOM;
  }
  update();
}
//----------------------------------------------------------------------------------------------------------------------

void NGLScene::keyPressEvent(QKeyEvent *_event)
{
  // this method is called every time the main window recives a key event.
  // we then switch on the key value and set the camera in the GLWindow
  switch (_event->key())
  {
  // escape key to quite
  case Qt::Key_Escape:
    QGuiApplication::exit(EXIT_SUCCESS);
    break;
  // turn on wirframe rendering
  case Qt::Key_W:
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    break;
  // turn off wire frame
  case Qt::Key_S:
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    break;
  // show full screen
  case Qt::Key_F:
    showFullScreen();
    break;
  // show windowed
  case Qt::Key_N:
    showNormal();
    break;
  case Qt::Key_Space:
    m_update ^= true;
    break;

  default:
    break;
  }
  // finally update the GLWindow and re-draw
  // if (isExposed())
  update();
}

void NGLScene::timerEvent(QTimerEvent *)
{
  if (m_update == false)
    return;
  auto updatebegin = std::chrono::steady_clock::now();
  m_grid->update(0.01f);
  auto updateend = std::chrono::steady_clock::now();
  // ngl::msg->addMessage(fmt::format("Update took {0} uS",std::chrono::duration_cast<std::chrono::microseconds> (updateend - updatebegin).count()));
  m_updateTime = std::chrono::duration_cast<std::chrono::microseconds>(updateend - updatebegin).count();

  update();
}
