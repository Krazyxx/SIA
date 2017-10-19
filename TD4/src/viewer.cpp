#include "viewer.h"

#include "glPrimitives.h"
#include "mesh.h"
#include "parser.h"
#include "areaLight.h"

#include <filesystem/resolver.h>
#include <nanogui/slider.h>
#include <nanogui/checkbox.h>
#include <nanogui/button.h>
#include <nanogui/layout.h>
#include <thread>

Viewer::Viewer() :
    nanogui::Screen(Vector2i(512,512+50), "Raytracer")
{
    m_drawCamera = false;
    m_drawRay = false;
    m_drawAABB = false;
    m_renderingDone = true;

    /* Add some UI elements to adjust the exposure value */
    using namespace nanogui;
    m_panel = new Widget(this);
    m_panel->setLayout(new BoxLayout(Orientation::Horizontal, Alignment::Middle, 10, 10));

    m_checkbox = new CheckBox(m_panel,"srgb");
    m_checkbox->setChecked(true);
    m_checkbox->setCallback(
        [&](bool value) {
            m_srgb = value ? 1 : 0;
        }
    );

    m_slider = new Slider(m_panel);
    m_slider->setValue(0.5f);
    m_slider->setFixedWidth(150);
    m_slider->setCallback(
        [&](float value) {
            m_scale = std::pow(2.f, (value - 0.5f) * 20);
        }
    );

    m_button = new Button(m_panel);
    m_button->setCaption("save PNG");
    m_button->setEnabled(false);
    m_button->setCallback(
        [&](void) {
            m_fbo.init(mFBSize,1);
            m_fbo.bind();
            drawContents();
            m_fbo.release();

            Bitmap img(Eigen::Vector2i(mFBSize.x(), mFBSize.y()-50*mPixelRatio));
            glPixelStorei(GL_PACK_ALIGNMENT, 1);
            m_fbo.bind();
            glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
            glReadPixels(0, 50*mPixelRatio, mFBSize.x(), mFBSize.y() + 50*mPixelRatio, GL_RGB, GL_FLOAT, img.data());
            m_fbo.release();

            std::string outputName = m_curentFilename;
            size_t lastdot = outputName.find_last_of(".");
            if (lastdot != std::string::npos)
                outputName.erase(lastdot, std::string::npos);
            outputName += ".png";

            img.save(outputName, true);

        }
    );

    m_panel->setSize(Eigen::Vector2i(512,50));
    performLayout(mNVGContext);
    m_panel->setPosition(Eigen::Vector2i((512 - m_panel->size().x()) / 2, 512));

    initializeGL();

    drawAll();
    setVisible(true);
}

Viewer::~Viewer()
{
    m_program.free();
    m_flatProgram.free();
}

void Viewer::initializeGL()
{
    std::cout << "Using OpenGL version: \"" << glGetString(GL_VERSION) << "\"" << std::endl;

    // load the default shaders
    m_program.initFromFiles("simple", DATA_DIR"/shaders/simple.vert", DATA_DIR"/shaders/simple.frag");
    m_flatProgram.initFromFiles("flat", DATA_DIR"/shaders/flat.vert", DATA_DIR"/shaders/flat.frag");
    m_tonemapProgram.initFromFiles("flat", DATA_DIR"/shaders/tonemap.vert", DATA_DIR"/shaders/tonemap.frag");

    MatrixXu indices(3, 2); /* Draw 2 triangles */
    indices.col(0) << 0, 1, 2;
    indices.col(1) << 2, 3, 0;

    MatrixXf positions(2, 4);
    positions.col(0) << 0, 0;
    positions.col(1) << 1, 0;
    positions.col(2) << 1, 1;
    positions.col(3) << 0, 1;

    m_tonemapProgram.bind();
    m_tonemapProgram.uploadIndices(indices);
    m_tonemapProgram.uploadAttrib("position", positions);

    /* Allocate texture memory for the rendered image */
    glGenTextures(1, &m_texture);
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // Assign camera to trackball
    m_trackball.setCamera(&m_GLCamera);
}

void Viewer::drawContents()
{
    if(m_resultImage) // raytracing in progress
    {
        /* Reload the partially rendered image onto the GPU */
        m_resultImage->lock();
        int borderSize = m_resultImage->getBorderSize();
        const Vector2i &size = m_resultImage->getSize();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_texture);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, m_resultImage->cols());
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, size.x(), size.y(),
                     0, GL_RGBA, GL_FLOAT, (uint8_t *) m_resultImage->data() +
                     (borderSize * m_resultImage->cols() + borderSize) * sizeof(Color4f));
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
        m_resultImage->unlock();

        glViewport(0, 50*mPixelRatio, mPixelRatio*size[0], mPixelRatio*size[1]);
        m_tonemapProgram.bind();
        m_tonemapProgram.setUniform("scale", m_scale);
        m_tonemapProgram.setUniform("srgb", m_srgb);
        m_tonemapProgram.setUniform("source", 0);
        m_tonemapProgram.drawIndexed(GL_TRIANGLES, 0, 2);
    }
    else if(m_scene) // scene loaded, OpenGL rendering
    {
        glEnable(GL_DEPTH_TEST);
        glViewport(0, 50*mPixelRatio, m_GLCamera.vpWidth()*mPixelRatio, m_GLCamera.vpHeight()*mPixelRatio);
        glClearColor(m_scene->backgroundColor()[0],m_scene->backgroundColor()[1],m_scene->backgroundColor()[2],1);
        glScissor(0, 50*mPixelRatio, m_GLCamera.vpWidth()*mPixelRatio, m_GLCamera.vpHeight()*mPixelRatio);
        glEnable(GL_SCISSOR_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDisable(GL_SCISSOR_TEST);

        m_program.bind();
        glUniformMatrix4fv(m_program.uniform("mat_view"), 1, GL_FALSE, m_GLCamera.viewMatrix().data());
        glUniformMatrix4fv(m_program.uniform("mat_proj"), 1, GL_FALSE, m_GLCamera.projectionMatrix().data());
        Vector3f lightDir = Vector3f(1,1,1).normalized();
        glUniform3fv(m_program.uniform("light_dir"),  1, lightDir.data());

        m_scene->draw();

        m_flatProgram.bind();
        glUniformMatrix4fv(m_flatProgram.uniform("mat_view"), 1, GL_FALSE, m_GLCamera.viewMatrix().data());
        glUniformMatrix4fv(m_flatProgram.uniform("mat_proj"), 1, GL_FALSE, m_GLCamera.projectionMatrix().data());

        if(m_drawCamera)
        {
            glUniform3f(m_flatProgram.uniform("color"),0.75f,0.75f,0.75f);
            m_scene->camera()->draw(&m_flatProgram);
        }

        if(m_drawAABB) // Draw bounding boxes
        {
            for(uint i=0; i<m_scene->shapeList().size(); ++i)
            {
                glUniformMatrix4fv(m_flatProgram.uniform("mat_obj"), 1, GL_FALSE, m_scene->shapeList()[i]->transformation().getMatrix().data());
                glUniform3f(m_flatProgram.uniform("color"),0.9f,0.9f,0.9f);
                Box::draw(&m_flatProgram, m_scene->shapeList()[i]->AABB());
            }
        }

        glUniformMatrix4fv(m_flatProgram.uniform("mat_obj"), 1, GL_FALSE, Eigen::Affine3f::Identity().data());

        if(m_drawRay)
        {
            glUniform3f(m_flatProgram.uniform("color"),0.9f,0.9f,0.9f);
            Line::draw(&m_flatProgram, m_ray.origin, m_ray.at(10.f));

            if(m_hit.foundIntersection())
            {
                Point3f x = m_ray.at(m_hit.t());
                Normal3f normal = m_hit.normal();

                glUniform3f(m_flatProgram.uniform("color"),0.9f,0.f,0.f);
                Point::draw(&m_flatProgram, x);
                Line::draw(&m_flatProgram, x, x + 0.25f * normal);

                // Sample material
                float etaA = m_hit.shape()->material()->etaA();
                float etaB = m_hit.shape()->material()->etaB();
                bool entering = -normal.dot(m_ray.direction) > 0;
                if(!entering) {
                    std::swap(etaA,etaB);
                    normal = -normal;
                }
                Vector3f r;
                if(refract(normal,m_ray.direction,etaA,etaB,r)) {
                    glUniform3f(m_flatProgram.uniform("color"),0.0f,0.9f,0.0f);
                    Line::draw(&m_flatProgram, x, x + 0.25f * r);
                }

                int n = 100;
                for(int i=0; i<n; ++i){
                    float pdf;
                    //Vector3f r = m_hit.object()->material()->us(normal,pdf);
                    Vector3f r = m_hit.shape()->material()->is(normal,-m_ray.direction,pdf);

                    glUniform3f(m_flatProgram.uniform("color"),0.f,0.8f,0.0f);
                    Line::draw(&m_flatProgram, x, x + 0.25f * r);
                }
            }
        }
    }

    glViewport(0, 0, mFBSize[0], mFBSize[1]);
}

void Viewer::framebufferSizeChanged()
{
    m_GLCamera.setViewport(mFBSize[0],mFBSize[1]-50);
    m_panel->setSize(Eigen::Vector2i(m_GLCamera.vpWidth(),50));
    performLayout(mNVGContext);
    m_panel->setPosition(Eigen::Vector2i((m_GLCamera.vpWidth() - m_panel->size().x()) / 2, m_GLCamera.vpHeight()));
}

void Viewer::loadScene(const std::string& filename)
{
    if(filename.size()>0) {
        filesystem::path path(filename);

        if (path.extension() != "scn")
            return;
        m_renderingDone = true;
        if(m_resultImage) {
            delete m_resultImage;
            m_resultImage = nullptr;
        }

        getFileResolver()->prepend(path.parent_path());

        ::Object* root = loadFromXML(filename);
        if (root->getClassType() == ::Object::EScene){
            if (m_scene)
                delete m_scene;
            m_scene = static_cast<Scene*>(root);
            for(uint i=0; i<m_scene->lightList().size(); ++i) {
                AreaLight *al = dynamic_cast<AreaLight*>(m_scene->lightList()[i]);
                if(al)
                    m_scene->addChild(al->shape());
            }
            m_scene->attachShaderToShapes(&m_program);
            m_scene->attachShaderToLights(&m_flatProgram);
            std::cout << m_scene->toString() << std::endl;
            m_curentFilename = filename;

            // Update GUI
            m_button->setEnabled(true);
            m_GLCamera = Camera(*m_scene->camera());
            setSize(m_GLCamera.outputSize() + Eigen::Vector2i(0,50));
            glfwSetWindowSize(glfwWindow(),m_GLCamera.vpWidth(),m_GLCamera.vpHeight()+50);
            m_panel->setSize(Eigen::Vector2i(m_GLCamera.vpWidth(),50));
            performLayout(mNVGContext);
            m_panel->setPosition(Eigen::Vector2i((m_GLCamera.vpWidth() - m_panel->size().x()) / 2, m_GLCamera.vpHeight()));
        }
        drawAll();
    }
}

void Viewer::loadImage(const filesystem::path &filename)
{
    m_curentFilename = filename.str();
    Bitmap bitmap(filename);
    m_resultImage = new ImageBlock(Eigen::Vector2i(bitmap.cols(), bitmap.rows()));
    m_resultImage->fromBitmap(bitmap);
    m_renderingDone = false;
    // Update GUI
    m_button->setEnabled(true);
    setSize(Eigen::Vector2i(m_resultImage->cols(), m_resultImage->rows()+50));
    glfwSetWindowSize(glfwWindow(),m_resultImage->cols(), m_resultImage->rows()+50);
    m_panel->setSize(Eigen::Vector2i(mFBSize[0],50));
    performLayout(mNVGContext);
    m_panel->setPosition(Eigen::Vector2i((mFBSize[0] - m_panel->size().x()) / 2, m_resultImage->rows()));
}

void renderBlock(Scene* scene, ImageBlock& block)
{
    const Camera *camera = scene->camera();

    float tanfovy2 = tan(camera->fovY()*0.5);
    Vector3f camX = camera->right() * tanfovy2 * camera->nearDist() * float(camera->vpWidth())/float(camera->vpHeight());
    Vector3f camY = camera->up() * tanfovy2 * camera->nearDist();
    Vector3f camF = camera->direction() * camera->nearDist();

    Integrator* integrator = scene->integrator();
    integrator->preprocess(scene);

    /* Clear the block contents */
    block.clear();

    Vector2i offset = block.getOffset();
    Vector2i size  = block.getSize();

    uint nbSample = camera->sampleCount(); //int(std::sqrt(camera->sampleCount()));

    /* For each pixel and pixel sample */
    for (int y=0; y<size.y(); ++y) {
        for (int x=0; x<size.x(); ++x) {
            // compute the primary ray parameters
            Ray ray;
            ray.origin = camera->position();

            Color3f radiance = Color3f::Zero();

            for(uint i=0; i<nbSample; ++i) {
                for(uint j=0; j<nbSample; ++j) {
                    float xx = 0.5f, yy = 0.5f;
                    if(camera->isStratified()) {
                        xx = Eigen::internal::random<float>(0,1);
                        yy = Eigen::internal::random<float>(0,1);
                    }
                    ray.direction = (camF + camX * (2.f * float(x + xx + offset[0])/float(camera->vpWidth())  - 1.)
                                      - camY * (2.f * float(y + yy + offset[1])/float(camera->vpHeight()) - 1.)).normalized();
                    radiance += integrator->Li(scene,ray);
                }
            }

            block.put(Vector2f(x+offset[0], y+offset[1]), radiance / (nbSample*nbSample));
        }
    }
}

void render(Scene* scene, ImageBlock* result, std::string outputName, bool* done)
{
    if(!scene)
        return;

    clock_t t = clock();
    Mesh::ms_itersection_count = 0;

    Vector2i outputSize = scene->camera()->outputSize();

    /* Create a block generator (i.e. a work scheduler) */
    BlockGenerator blockGenerator(outputSize, BLOCK_SIZE);

    int blockCount = blockGenerator.getBlockCount();

    Integrator* integrator = scene->integrator();
    integrator->preprocess(scene);

    #pragma omp parallel for
    for(int i=0; i<blockCount; i++) {
        /* Allocate memory for a small image block to be rendered by the current thread */
        ImageBlock block(Vector2i(BLOCK_SIZE,BLOCK_SIZE));

        /* Request an image block from the block generator */
        blockGenerator.next(block);

        /* Render all contained pixels */
        renderBlock(scene,block);

        /* The image block has been processed. Now add it to
           the "big" block that represents the entire image */
        result->put(block);
    }

    t = clock() - t;
    std::cout << "Raytracing time : " << float(t)/CLOCKS_PER_SEC << "s  -  nb triangle intersection: " << Mesh::ms_itersection_count << "\n";

    Bitmap* img = result->toBitmap();
//    img->save(outputName);
    delete img;

    *done = true;
}

bool Viewer::keyboardEvent(int key, int scancode, int action, int modifiers)
{
    if(action == GLFW_PRESS) {
        switch(key)
        {
        case GLFW_KEY_L:
        {
            std::string filename = nanogui::file_dialog( { {"scn", "Scene file"}, {"exr", "Image file"} }, false);
            filesystem::path path(filename);
            if (path.extension() == "scn")
                loadScene(filename);
            else if(path.extension() == "exr" || path.extension() == "png")
                loadImage(filename);
            return true;
        }
        case GLFW_KEY_C:
        {
            if(m_scene) {
                m_scene->setCamera(new Camera(m_GLCamera));
                drawAll();
            }
            return true;
        }
        case GLFW_KEY_H:
        {
            m_drawCamera = !m_drawCamera;
            drawAll();
            return true;
        }
        case GLFW_KEY_R:
        {
            if(m_scene && m_renderingDone) {
                m_renderingDone = false;
                /* Determine the filename of the output bitmap */
                std::string outputName = m_curentFilename;
                size_t lastdot = outputName.find_last_of(".");
                if (lastdot != std::string::npos)
                    outputName.erase(lastdot, std::string::npos);
                outputName += ".exr";

                /* Allocate memory for the entire output image */
                if(m_resultImage)
                    delete m_resultImage;
                m_resultImage = new ImageBlock(m_scene->camera()->outputSize());

                std::thread render_thread(render,m_scene,m_resultImage,outputName,&m_renderingDone);
                render_thread.detach();
            }
            return true;
        }
        case GLFW_KEY_B:
        {
            m_drawAABB = !m_drawAABB;
            drawAll();
            return true;
        }
        case GLFW_KEY_ESCAPE:
            exit(0);
        default:
            break;
        }
    }
    return Screen::keyboardEvent(key,scancode,action,modifiers);
}

bool Viewer::dropEvent(const std::vector<std::string> &filenames)
{
    // only tries to load the first file
    filesystem::path path(filenames.front());
    if (path.extension() == "scn")
        loadScene(filenames.front());
    else if(path.extension() == "exr" || path.extension() == "png")
        loadImage(filenames.front());

    drawAll();
    return true;
}

void Viewer::select(const Point2i &point)
{
    Point3f orig;
    Vector3f dir;
    m_GLCamera.convertClickToLine(point,orig,dir);
    m_ray.origin = orig;
    m_ray.direction = dir;
    m_hit = Hit();
    m_scene->intersect(m_ray,m_hit);
    m_drawRay = true;
    drawAll();
}

bool Viewer::mouseButtonEvent(const Eigen::Vector2i &p, int button, bool down, int modifiers)
{
    if(Widget::mouseButtonEvent(p,button,down,modifiers))
        return true;

    if(down) {

        if(m_renderingDone && m_resultImage) {
            delete m_resultImage;
            m_resultImage = nullptr;
        }

        m_mouseCoords = p;
        bool fly = (modifiers == GLFW_MOD_ALT);
        bool shift = (modifiers == GLFW_MOD_SHIFT);
        switch(button)
        {
        case GLFW_MOUSE_BUTTON_LEFT:
            if(shift){
                select(p);
            }
            else if(fly)
            {
                m_currentTrackingMode = TM_LOCAL_ROTATE;
                m_trackball.start(Trackball::Local);
                m_trackball.track(m_mouseCoords);
            }
            else
            {
                m_currentTrackingMode = TM_ROTATE_AROUND;
                m_trackball.start(Trackball::Around);
                m_trackball.track(m_mouseCoords);
            }
            break;
        case GLFW_MOUSE_BUTTON_MIDDLE:
            if(fly)
                m_currentTrackingMode = TM_FLY_Z;
            else
                m_currentTrackingMode = TM_ZOOM;
            break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            m_currentTrackingMode = TM_FLY_PAN;
            break;
        default:
            break;
        }
    }else{
        m_currentTrackingMode = TM_NO_TRACK;
    }
    drawAll();
    return true;
}

bool Viewer::scrollEvent(const Eigen::Vector2i &p, const Eigen::Vector2f &rel)
{
    if(Widget::scrollEvent(p,rel))
        return true;

    m_GLCamera.zoom(rel[1]*0.01);

    drawAll();
    return true;
}

bool Viewer::mouseMotionEvent(const Eigen::Vector2i &p, const Eigen::Vector2i &rel, int button, int modifiers)
{
    if(Widget::mouseMotionEvent(p,rel,button,modifiers))
        return true;

    // tracking
    if(m_currentTrackingMode != TM_NO_TRACK)
    {
        float dx =   float(p[0] - m_mouseCoords[0]) / float(m_GLCamera.vpWidth());
        float dy = - float(p[1] - m_mouseCoords[1]) / float(m_GLCamera.vpHeight());

        // speedup the transformations
        if(modifiers == GLFW_MOD_SHIFT)
        {
            dx *= 10.;
            dy *= 10.;
        }

        switch(m_currentTrackingMode)
        {
        case TM_ROTATE_AROUND:
        case TM_LOCAL_ROTATE:
            m_trackball.track(p);
            break;
        case TM_ZOOM :
            m_GLCamera.zoom(dy*100);
            break;
        case TM_FLY_Z :
            m_GLCamera.localTranslate(Vector3f(0, 0, -dy*200));
            break;
        case TM_FLY_PAN :
            m_GLCamera.localTranslate(Vector3f(dx*200, dy*200, 0));
            break;
        default:
            break;
        }

        drawAll();
    }

    m_mouseCoords = p;
    return true;
}
