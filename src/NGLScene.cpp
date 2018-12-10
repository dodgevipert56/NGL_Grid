#include <QMouseEvent>
#include <QGuiApplication>

#include "NGLScene.h"
#include <ngl/NGLInit.h>
#include <ngl/VAOPrimitives.h> // methods to create primitives - torus, sphere, and built in prims
#include <ngl/ShaderLib.h> // another singleton class, ways of managing itself
#include <ngl/NGLStream.h> // implements all ostream operators, for printing
#include <iostream>
#include <ngl/Random.h>
#include <ngl/fmt/format.h>
#include <ngl/VAOFactory.h>
#include <ngl/SimpleVAO.h>
#include <ngl/NGLMessage.h>


NGLScene::NGLScene(size_t _numMeshes)
{
  // re-size the widget to that of the parent (in this case the GLFrame passed in on construction)
  setTitle(fmt::format("Blank NGL Num Meshes {0}", _numMeshes).c_str()); //fmt - lib python and c style formats with place holder {0} or %d stuff

  m_meshes.resize(_numMeshes);
  createMeshes(); // call createMeshes()
  m_collection.resize(4);
  updateCollection();
  startTimer(10);
}

// this method is created so keypress can add more meshes
void NGLScene::addMesh(MeshType _m)
{
    ngl::Random *rng = ngl::Random::instance();
    MeshData m;
    m.pos = rng->getRandomPoint(40, 0, 40); // on a plane the position with given range 40 in this case
    m.rot.m_y =rng->randomPositiveNumber(360.0f); // 0 to 360
    m.scale.set(1.0f, 1.0f, 1.0f);
    m.colour = rng->getRandomColour4();
    m.type = _m;
    m_meshes.push_back(m);
}

void NGLScene::createMeshes() // not const becuz it a mutate
{
    ngl::Random *rng = ngl::Random::instance(); // rand class
    for (auto &m : m_meshes) // its going to mutate the value in there and need a copy
    {
        m.pos = rng->getRandomPoint(40, 0, 40); // on a plane the position with given range 40 in this case
        m.rot.m_y =rng->randomPositiveNumber(360.0f); // 0 to 360
        m.scale.set(1.0f, 1.0f, 1.0f);
        m.colour = rng->getRandomColour4();
        int type = static_cast<int>(rng->randomPositiveNumber(4));
        switch(type)
        {
            case 0 : m.type = MeshType::TEAPOT; break;
            case 1 : m.type = MeshType::CUBE; break;
            case 2 : m.type = MeshType::SPHERE; break;
            case 3 : m.type = MeshType::TROLL; break;   // resize the struct iterate through and populate these

        }
    }
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
                               0.5f, 200.0f); //FOV , last are near and far clipping planes

}

constexpr auto *ColourShader = "ColourShader"; // In compile time replaced
constexpr auto *LineShader = "LineShader";

void NGLScene::initializeGL()
{
  // we need to initialise the NGL lib which will load all of the OpenGL functions, this must
  // be done once we have a valid GL context but before we call any GL commands. If we dont do
  // this everything will crash
  ngl::NGLInit::instance();
  glClearColor(0.8f, 0.8f, 0.8f, 1.0f);			   // Grey Background- BACKGROUND
  // enable depth testing for drawing
  glEnable(GL_DEPTH_TEST);
  // enable multisampling for smoother drawing
  glEnable(GL_MULTISAMPLE);

  ngl::VAOPrimitives::instance()->createSphere("sphere", 1.0f, 40); // initialise sphere with descriptions
  ngl::ShaderLib *shader = ngl::ShaderLib::instance();
  shader->loadShader(ColourShader, "shaders/ColourVertex.glsl", //loading the shaders from the ones we created
                      "shaders/ColourFragment.glsl");

  shader->loadShader(LineShader, "shaders/ColourLineVertex.glsl", // later switch btw line  and colour shader pressing W  and S
                    "shaders/ColourLineFragment.glsl");


  m_view = ngl::lookAt({0.0f, 20.0f, 20.0f},  //gen a func to simulate glu lookat, 4*4 matrix
                       ngl::Vec3::zero(),   // return a 0 matrix
                       ngl::Vec3::up());    // return a 0 matrix

  m_vao = ngl::VAOFactory::createVAO(ngl::simpleVAO, GL_LINES); // created Vertex array objects
}

void NGLScene::loadMatrixToShader(const ngl::Mat4 &_tx, const ngl::Vec4 &_colour) // getMatrix is a const cant be mutable or something like that
{
    ngl::ShaderLib *shader = ngl::ShaderLib::instance();
    shader->use(ColourShader); //activate shader
    //shader->use(LineShader);

    //shader->setUniform("MVP", ngl::Mat4(0.2)); // careful of what Mat4

    shader->setUniform("MVP", m_project*m_view*_tx); // using proj and view identity matrices from NGLScene
/*
    std::cout<<m_view<<"\n"<<m_project<<"\n"<<_tx<<"\n"
            <<m_project*m_view*_tx<<"\n";
    std::cout << "******************" << "\n";
*/
    shader->setUniform("vertColour", _colour);
}


void NGLScene::loadMatrixToLineShader(const ngl::Mat4 &_tx) // getMatrix is a const cant be mutable or something like that
{
    ngl::ShaderLib *shader = ngl::ShaderLib::instance();

    shader->use(LineShader); //activate shader
    shader->setUniform("MVP", m_project*m_view*_tx); // using proj and view identity matrices from NGLScene
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
  auto *prim = ngl::VAOPrimitives::instance();
  for(auto m : m_meshes)
  {
      tx.setPosition(m.pos);
      tx.setRotation(m.rot);
      tx.setScale(m.scale);
      loadMatrixToShader(mouseRotation*tx.getMatrix(), m.colour);

      switch(m.type)
      {
        case MeshType::TEAPOT : prim->draw("teapot"); break;
        case MeshType::CUBE : prim->draw("cube"); break;
        case MeshType::SPHERE : prim->draw("sphere"); break;
        case MeshType::TROLL : prim->draw("troll"); break;
      }
   }


   if(m_drawLines==true)
   {
        loadMatrixToLineShader(mouseRotation*tx.getMatrix());
        drawLines(mouseRotation); // only same values
    }

  /*
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
    */
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
  case Qt::Key_L : m_drawLines^=true; break;
  case Qt::Key_C : if(m_meshes.size()<=0) m_meshes.resize(100); createMeshes(); break; // draw each time, even after all gone

  case Qt::Key_1 : addMesh(MeshType::TEAPOT); break;
  case Qt::Key_2 : addMesh(MeshType::CUBE); break;
  case Qt::Key_3 : addMesh(MeshType::SPHERE); break;
  case Qt::Key_4 : addMesh(MeshType::TROLL); break;



  default : break;
  }
  // finally update the GLWindow and re-draw

    update(); //QT event
}

void NGLScene::updateCollection()
{
    for(auto &c : m_collection)
        c.clear();


    for(auto &m : m_meshes) // need memory otherwise we dont get a draw - not a copy
    {
        switch(m.type)
        {
            case MeshType::TEAPOT : m_collection[0].push_back(&m); break;
            case MeshType::CUBE : m_collection[1].push_back(&m); break;
            case MeshType::SPHERE : m_collection[2].push_back(&m); break;
            case MeshType::TROLL : m_collection[3].push_back(&m); break;
        }
    }
}

void NGLScene::drawLines(const ngl::Mat4 & _tx)
{
    std::vector<Vertex> line; // size of vertex and give the data structure

    // Color lines
    const std::array<ngl::Vec4, 4> colours = {{
        ngl::Vec4(1.0f, 0.0f, 0.0f), // red teapot
        ngl::Vec4(0.0f, 1.0f, 0.0f), // green cube
        ngl::Vec4(0.0f, 0.0f, 1.0f), // blue sphere
        ngl::Vec4(1.0f, 1.0f, 1.0f), // white troll
    }};
    /*
    //std::vector<MeshData *> teapots; //raw pointer

    m_collection[0].clear(); // get rid of all the elements and set it back to 0

    for(auto &m : m_meshes) // need memory otherwise we dont get a draw - not a copy
    {
        // connect teapots to other teapots
        if(m.type == MeshType::TEAPOT)
        {
            m_collection[0].push_back(&m);
        }
    }
    */
    Vertex a,b;

    for(size_t i=0; i<m_collection.size(); ++i)
    {

        a.colour = colours[i] ;
        b.colour = colours[i];
        auto size = m_collection[i].size();

        for(size_t original = 0; original < size; ++original) //asymtotic
        {
            for(size_t current = 0; current < size; ++current)
            {
                if(original == current) continue;
                a.pos.set(m_collection[i][original]->pos);
                b.pos.set(m_collection[i][current]->pos);
                line.push_back(a);
                line.push_back(b);

            }
        }
    }

    std::cout <<"num teapots" << m_collection[0].size()<< "num meshes" << m_meshes.size()<<"\n";

    /*
    line[0].pos.set(-10,1,0);
    line[1].pos.set(10,1,0);
    line[0].colour.set(1.0f, 0.0f, 0.0f, 1.0f);
    line[1].colour.set(0.0f, 1.0f, 0.0f, 1.0f);
    */

    m_vao->bind();
    //simpleVao is a string
    m_vao->setData(ngl::SimpleVAO::VertexData(line.size()*sizeof(Vertex), line[0].pos.m_x)); // how big is data and where it starts

    m_vao->setVertexAttributePointer(0, 3, GL_FLOAT, sizeof(Vertex), 0); // connecting a line between 2 objects
    m_vao->setVertexAttributePointer(1, 4, GL_FLOAT, sizeof(Vertex), 3);
    m_vao->setNumIndices(line.size());
    loadMatrixToLineShader(_tx);
    m_vao->draw();
    m_vao->unbind();

    m_vao->unbind();
}

// ERASING THE OBJECTS HAPPEN HERE
void NGLScene::prune()
{
    for(auto it = m_meshes.begin(); it != m_meshes.end();)
    {
        if(it->distance < 0.05f)
        {
            it = m_meshes.erase(it);
        }
        else
        {
            ++it; //itereat through all meshes
        }
    }
}


void NGLScene::timerEvent(QTimerEvent *_event)
{
    //ngl::NGLMessage::addMessage("timer", Colours::YELLOW, TimeFormat::TIMEDATE);
    updateCollection();

    for(size_t i=0; i<m_collection.size(); ++i)
    {
        ngl::Vec3 center;

        for(auto m:m_collection[i]) // all of the teapots
            center += m->pos;

        center /= m_collection[i].size(); // doing this gives the center

        std::cout << "Center is " << center << "\n";

        // get into the center of each teapots,  average center between teapots and not the center of grid or origin
        for(auto &m:m_collection[i])
        {
            m->dir =center - m->pos; //length between the two objects
            m->distance = m->dir.length(); //overall distance

            std::cout << "Length: " << m->distance << "\n";

            m->dir.normalize();
            m->pos+=m->dir * 0.1f;
        }
    }
    prune();
    update();
}

/*
for(size_t i=0; i<m_collection.size(); ++i)
{
for(auto m:m_collection[i]) // all of the teapots
{
    center += m->pos;

     center /= m_collection[i].size(); // doing this gives the center

std::cout << "Center is " << center << "\n";

// get into the center of each teapots,  average center between teapots and not the center of grid or origin
for(auto &m:m_collection[i])
{
    m->dir =center - m->pos;
    m->dir.normalize();
    m->pos+=m->dir * 0.01f;
}
}
}
*/
