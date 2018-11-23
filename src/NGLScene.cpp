#include <QMouseEvent>
#include <QGuiApplication>

#include "NGLScene.h"
#include <ngl/NGLInit.h>
#include <ngl/VAOPrimitives.h> // methods to create primitives - torus, sphere, and built in prims
#include <ngl/ShaderLib.h> // another singleton class, ways of managing itself
#include <ngl/NGLStream.h> // implements all ostream operators, for printing
#include <iostream>

NGLScene::NGLScene()
{
  // re-size the widget to that of the parent (in this case the GLFrame passed in on construction)
  setTitle("Blank NGL");
}


NGLScene::~NGLScene()
{
  std::cout<<"Shutting down NGL, removing VAO's and Shaders\n";
}



void NGLScene::resizeGL(int _w , int _h)
{
  m_win.width  = static_cast<int>( _w * devicePixelRatio() );
  m_win.height = static_cast<int>( _h * devicePixelRatio() );
  m_project = ngl::perspective(45.0f, static_cast<float>(_w)/_h,
                               0.5f, 10.0f); //FOV , last are near and far clipping planes

}

constexpr char *ColourShader = "Colour Shader"; //compile time replaced

void NGLScene::initializeGL()
{
  // we need to initialise the NGL lib which will load all of the OpenGL functions, this must
  // be done once we have a valid GL context but before we call any GL commands. If we dont do
  // this everything will crash
  ngl::NGLInit::instance();
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);			   // Grey Background
  // enable depth testing for drawing
  glEnable(GL_DEPTH_TEST);
  // enable multisampling for smoother drawing
  glEnable(GL_MULTISAMPLE);

  ngl::VAOPrimitives::instance()->createSphere("sphere", 1.0f, 40); // initialise sphere with descriptions
  ngl::ShaderLib *shader = ngl::ShaderLib::instance();
  shader->loadShader(ColourShader, "shaders/ColourVertex.glsl",
                      "shaders/ColourFragment.glsl");

  m_view = ngl::lookAt({0.0f, 2.0f, 2.0f},  //gen a func sim to glu lookat, 4*4 matrix
                       ngl::Vec3::zero(),   // return a 0 matrix
                       ngl::Vec3::up());    // return a 0 mat

}

void NGLScene::loadMatrixToShader(const ngl::Mat4 &_tx, const ngl::Vec4 &_colour) // getMatrix is a const cant be mutable or something like that
{
    ngl::ShaderLib *shader = ngl::ShaderLib::instance();
    shader->use(ColourShader); //activate shader
    //shader->setUniform("MVP", ngl::Mat4(0.2)); // careful of what Mat4

    shader->setUniform("MVP", m_project*m_view*_tx); // using proj and view identity matrices from NGLScene

    std::cout<<m_view<<"\n"<<m_project<<"\n"<<_tx<<"\n"
            <<m_project*m_view*_tx<<"\n";
    std::cout << "******************" << "\n";

    shader->setUniform("vertColour", _colour);
}


void NGLScene::paintGL()
{
  // clear the screen and depth buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glViewport(0,0,m_win.width,m_win.height);

  ngl::Mat4 rotX;
  ngl::Mat4 rotY;
  ngl::Mat4 mouseRotation;
  rotX.rotateX(m_win.spinXFace); // rot around x axis and in angles according to mouse press down
  rotY.rotateY(m_win.spinYFace);
  mouseRotation = rotY*rotX;

  ngl::Transformation tx;
  tx.setScale(0.2f, 0.2f, 0.2f);

  for(float z = -20.0f; z < 20.0f; z+=0.5)
  {
    for(float x = -20.0f; x < 20.0f; x+=0.5)
    {
        ngl::Vec4 colour(z,z,x,1.0f);
        colour.normalize();
        tx.setPosition(x, 0.0f, z);
        //tx.setRotation(45, 22, 18);

        // initialise the MVP matrix everytime you draw
        loadMatrixToShader( mouseRotation * tx.getMatrix(), colour); // (tx.getMatrix() * mouseRotation) // calling new loadMatrix - does local rotation on object
        ngl::VAOPrimitives::instance()->draw("teapot"); // draw simple teapot
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------

void NGLScene::keyPressEvent(QKeyEvent *_event)
{
  // this method is called every time the main window recives a key event.
  // we then switch on the key value and set the camera in the GLWindow
  switch (_event->key())
  {
  // escape key to quite
  case Qt::Key_Escape : QGuiApplication::exit(EXIT_SUCCESS); break;
  case Qt::Key_Space :
      m_win.spinXFace=0;
      m_win.spinYFace=0;
      m_modelPos.set(ngl::Vec3::zero());

  break;
  case Qt::Key_W : glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); break; // wireframe draw
  case Qt::Key_S : glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); break; // solid draw


  default : break;
  }
  // finally update the GLWindow and re-draw

    update();
}
