#ifndef NGLSCENE_H_
#define NGLSCENE_H_
#include <ngl/Vec3.h>
#include "WindowParams.h"
#include <ngl/Transformation.h> // pos rot and scale
#include <ngl/Mat4.h>
#include <ngl/AbstractVAO.h>
// this must be included after NGL includes else we get a clash with gl libs
#include <QOpenGLWindow>
//----------------------------------------------------------------------------------------------------------------------
/// @file NGLScene.h
/// @brief this class inherits from the Qt OpenGLWindow and allows us to use NGL to draw OpenGL
/// @author Jonathan Macey
/// @version 1.0
/// @date 10/9/13
/// Revision History :
/// This is an initial version used for the new NGL6 / Qt 5 demos
/// @class NGLScene
/// @brief our main glwindow widget for NGL applications all drawing elements are
/// put in this file
//----------------------------------------------------------------------------------------------------------------------

class NGLScene : public QOpenGLWindow
{
  public:
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief ctor for our NGL drawing class
    /// @param [in] parent the parent window to the class
    //----------------------------------------------------------------------------------------------------------------------
    NGLScene(size_t _numMeshes);
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief dtor must close down ngl and release OpenGL resources
    //----------------------------------------------------------------------------------------------------------------------
    ~NGLScene() override;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief the initialize class is called once when the window is created and we have a valid GL context
    /// use this to setup any default GL stuff
    //----------------------------------------------------------------------------------------------------------------------
    void initializeGL() override;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief this is called everytime we want to draw the scene
    //----------------------------------------------------------------------------------------------------------------------
    void paintGL() override;
    void timerEvent(QTimerEvent *_event) override;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief this is called everytime we resize the window
    //----------------------------------------------------------------------------------------------------------------------
    void resizeGL(int _w, int _h) override;

private:

    //----------------------------------------------------------------------------------------------------------------------
    /// @brief Qt Event called when a key is pressed
    /// @param [in] _event the Qt event to query for size etc
    //----------------------------------------------------------------------------------------------------------------------
    void keyPressEvent(QKeyEvent *_event) override;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief this method is called every time a mouse is moved
    /// @param _event the Qt Event structure
    //----------------------------------------------------------------------------------------------------------------------
    void mouseMoveEvent (QMouseEvent * _event ) override;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief this method is called everytime the mouse button is pressed
    /// inherited from QObject and overridden here.
    /// @param _event the Qt Event structure
    //----------------------------------------------------------------------------------------------------------------------
    void mousePressEvent ( QMouseEvent *_event) override;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief this method is called everytime the mouse button is released
    /// inherited from QObject and overridden here.
    /// @param _event the Qt Event structure
    //----------------------------------------------------------------------------------------------------------------------
    void mouseReleaseEvent ( QMouseEvent *_event ) override;

    //----------------------------------------------------------------------------------------------------------------------
    /// @brief this method is called everytime the mouse wheel is moved
    /// inherited from QObject and overridden here.
    /// @param _event the Qt Event structure
    //----------------------------------------------------------------------------------------------------------------------
    void wheelEvent( QWheelEvent *_event) override;
    /// @brief windows parameters for mouse control etc.
    WinParams m_win;
    /// position for our model
    ngl::Vec3 m_modelPos;

    ngl::Mat4 m_view;
    ngl::Mat4 m_project;
    void loadMatrixToShader(const ngl::Mat4 &_tx, const ngl::Vec4 &_colour);
    void loadMatrixToLineShader(const ngl::Mat4 &_tx);

    enum class MeshType : char {TEAPOT, CUBE, SPHERE, TROLL}; // make it char to take less data

    struct MeshData
    {

        ngl::Vec3 pos;   // no need _ cuz of lite data structures
        ngl::Vec3 dir;
        ngl::Vec3 scale;
        ngl::Vec3 rot;    // all this is a pod type
        ngl::Vec4 colour;
        MeshType type;
        float distance;

        MeshData(const ngl::Vec3 &_pos, const ngl::Vec3 &_scale, // constructor intialisation
                 const ngl::Vec3 &_rot, const ngl::Vec4 &_colour,
                 MeshType _type) :
            pos(_pos), scale(_scale), rot(_rot), colour(_colour),
            type(_type){}
        MeshData() = default;
        MeshData(const MeshData &) = default;
        ~MeshData() = default;

    };

    // no need to use smart pointers since its a pod type therefore can construct and destruct
    std::vector<MeshData> m_meshes; // making object or instance
    std::vector<std::vector<MeshData *>> m_collection;
    void createMeshes(); // not const becuz it a mutate

    struct Vertex // represent vertex
    {
        ngl::Vec3 pos;
        ngl::Vec4 colour;
    };

    void drawLines(const ngl::Mat4 & _tx);

    void updateCollection();

    void prune();
    std::unique_ptr<ngl::AbstractVAO> m_vao; // use this to populate data and draw

    void addMesh(MeshType _m);
    bool m_drawLines = true;


};



#endif
