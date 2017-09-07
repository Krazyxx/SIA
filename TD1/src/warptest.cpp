/*
    This file is part of Nori, a simple educational ray tracer

    Copyright (c) 2015 by Wenzel Jakob

    Nori is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License Version 3
    as published by the Free Software Foundation.

    Nori is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "warp.h"
#include <nanogui/screen.h>
#include <nanogui/glutil.h>
#include <nanogui/label.h>
#include <nanogui/window.h>
#include <nanogui/layout.h>
#include <nanogui/entypo.h>
#include <nanogui/combobox.h>
#include <nanogui/slider.h>
#include <nanogui/textbox.h>
#include <nanogui/checkbox.h>
#include <nanogui/messagedialog.h>
#include <hypothesis.h>
#include <random>
#include <memory>

/* =======================================================================
 *   WARNING    WARNING    WARNING    WARNING    WARNING    WARNING
 * =======================================================================
 *   Remember to put on SAFETY GOGGLES before looking at this file. You
 *   are most certainly not expected to read or understand any of it.
 * ======================================================================= */

#if defined(_MSC_VER)
#  pragma warning (disable: 4305 4244)
#endif

using namespace nanogui;
using namespace std;

/*
 * Hammersley point sets. Deterministic and look random.
 * Base p = 2, which is especially fast for computation.
 */
void PlaneHammersley(float *result, int n)
{
  float p, u, v;
  int k, kk, pos;
  
  for (k=0, pos=0 ; k<n ; k++)
  {
    u = 0;
    for (p=0.5, kk=k ; kk ; p*=0.5, kk>>=1)
      if (kk & 1)                           // kk mod 2 == 1
           u += p;

    v = (k + 0.5) / n;

    result[pos++] = u;
    result[pos++] = v;
  }
}

/*
 * Halton point set generation
 * two p-adic Van der Corport sequences
 * Useful for incremental approach.
 * p1 = 2, p2 = 3(default)
 */
void PlaneHalton(float *result, int n, int p2)
{
  float p, u, v, ip;
  int k, kk, pos, a;
  
  for (k=0, pos=0 ; k<n ; k++)
  {
    u = 0;
    for (p=0.5, kk=k ; kk ; p*=0.5, kk>>=1)
      if (kk & 1)                          // kk mod 2 == 1
    u += p;

    v = 0;
    ip = 1.0/p2;                           // recipical of p2
    for (p=ip, kk=k ; kk ; p*=ip, kk/=p2)  // kk = (int)(kk/p2)
      if ((a = kk % p2))
    v += a * p;

    result[pos++] = u;
    result[pos++] = v;
  }
}

class WarpTest : public nanogui::Screen {
public:
    enum PointType {
        Independent = 0,
        Grid,
        Stratified,
        Hammersley,
        Halton
    };

    enum WarpType {
        Square = 0,
        Disk,
        UniformHemisphere,
        CosineHemisphere
    };

    WarpTest(): Screen(nanogui::Vector2i(800, 600), "Sampling and Warping") {
        initializeGUI();
        m_drawHistogram = false;

        gen = std::default_random_engine(rd());
        dist = std::uniform_real_distribution<float>(0,1);
    }

    std::pair<Point3f, float> warpPoint(WarpType warpType, const Point2f &sample) {
        Point3f result;

        switch (warpType) {
            case Square: result << Warp::squareToUniformSquare(sample), 0; break;
            case Disk: result << Warp::squareToUniformDisk(sample), 0; break;
            case UniformHemisphere: result << Warp::squareToUniformHemisphere(sample); break;
            case CosineHemisphere: result << Warp::squareToCosineHemisphere(sample); break;
        }

        return std::make_pair(result, 1.f);
    }

    void generatePoints(int &pointCount, PointType pointType, WarpType warpType,
                        MatrixXf &positions, MatrixXf &weights) {
        /* Determine the number of points that should be sampled */
        int sqrtVal = (int) (std::sqrt((float) pointCount) + 0.5f);
        float invSqrtVal = 1.f / sqrtVal;
        if (pointType == Grid || pointType == Stratified || pointType == Halton || pointType == Hammersley)
            pointCount = sqrtVal*sqrtVal;

        positions.resize(3, pointCount);
        weights.resize(1, pointCount);

        float* point;
        if(pointType == Hammersley) {
            point = new float[2*pointCount];
            PlaneHammersley(point, pointCount);
        }else if(pointType == Halton) {
            point = new float[2*pointCount];
            PlaneHalton(point, pointCount, 3);
        }

        for (int i=0; i<pointCount; ++i) {
            int y = i / sqrtVal, x = i % sqrtVal;
            Point2f sample;

            switch (pointType) {
                case Independent:
                    sample = Point2f(dist(gen), dist(gen));
                    break;

                case Grid:
                    sample = Point2f((x + 0.5f) * invSqrtVal, (y + 0.5f) * invSqrtVal);
                    break;

                case Stratified:
                    sample = Point2f((x + dist(gen)) * invSqrtVal,
                                     (y + dist(gen)) * invSqrtVal);
                    break;
                case Hammersley:
                case Halton:
                    sample = Point2f(point[2*i], point[2*i+1]);
            }

            auto result = warpPoint(warpType, sample);
            positions.col(i) = result.first;
            weights(0, i) = result.second;
        }
        if(pointType == Halton || pointType == Hammersley) {
            delete[] point;
        }
    }

    void refresh() {
        PointType pointType = (PointType) m_pointTypeBox->selectedIndex();
        WarpType warpType = (WarpType) m_warpTypeBox->selectedIndex();
        m_pointCount = (int) std::pow(2.f, 15 * m_pointCountSlider->value() + 5);

        /* Generate the point positions */
        MatrixXf positions, values;
        try {
            generatePoints(m_pointCount, pointType, warpType, positions, values);
        } catch (const RTException &e) {
            m_warpTypeBox->setSelectedIndex(0);
            refresh();
            new nanogui::MessageDialog(this, nanogui::MessageDialog::Type::Warning, "Error", "An error occurred: " + std::string(e.what()));
            return;
        }

        float value_scale = 0.f;
        for (int i=0; i<m_pointCount; ++i)
            value_scale = std::max(value_scale, values(0, i));
        value_scale = 1.f/value_scale;

        if (warpType != Square) {
            for (int i=0; i<m_pointCount; ++i) {
                if (values(0, i) == 0.0f) {
                    positions.col(i) = Eigen::Vector3f::Constant(std::numeric_limits<float>::quiet_NaN());
                    continue;
                }
                positions.col(i) =
                    ((value_scale == 0 ? 1.0f : (value_scale * values(0, i))) *
                     positions.col(i)) * 0.5f + Eigen::Vector3f(0.5f, 0.5f, 0.0f);
            }
        }

        /* Generate a color gradient */
        MatrixXf colors(3, m_pointCount);
        float colScale = 1.f / m_pointCount;
        for (int i=0; i<m_pointCount; ++i)
            colors.col(i) << i*colScale, 1-i*colScale, 0;

        /* Upload points to GPU */
        m_pointShader->bind();
        m_pointShader->uploadAttrib("position", positions);
        m_pointShader->uploadAttrib("color", colors);

        /* Upload lines to GPU */
        if (m_gridCheckBox->checked()) {
            int gridRes = (int) (std::sqrt((float) m_pointCount) + 0.5f);
            int fineGridRes = 16*gridRes, idx = 0;
            m_lineCount = 4 * (gridRes+1) * (fineGridRes+1);
            positions.resize(3, m_lineCount);
            float coarseScale = 1.f / gridRes, fineScale = 1.f / fineGridRes;
            for (int i=0; i<=gridRes; ++i) {
                for (int j=0; j<=fineGridRes; ++j) {
                    auto pt = warpPoint(warpType, Point2f(j     * fineScale, i * coarseScale));
                    positions.col(idx++) = value_scale == 0.f ? pt.first : (pt.first * pt.second * value_scale);
                    pt = warpPoint(warpType, Point2f((j+1) * fineScale, i * coarseScale));
                    positions.col(idx++) = value_scale == 0.f ? pt.first : (pt.first * pt.second * value_scale);
                    pt = warpPoint(warpType, Point2f(i*coarseScale, j     * fineScale));
                    positions.col(idx++) = value_scale == 0.f ? pt.first : (pt.first * pt.second * value_scale);
                    pt = warpPoint(warpType, Point2f(i*coarseScale, (j+1) * fineScale));
                    positions.col(idx++) = value_scale == 0.f ? pt.first : (pt.first * pt.second * value_scale);
                }
            }
            if (warpType != Square) {
                for (int i=0; i<m_lineCount; ++i)
                    positions.col(i) = positions.col(i) * 0.5f + Eigen::Vector3f(0.5f, 0.5f, 0.0f);
            }
            m_gridShader->bind();
            m_gridShader->uploadAttrib("position", positions);
        }

        int ctr = 0;
        positions.resize(3, 106);
        for (int i=0; i<=50; ++i) {
            float angle1 = i * 2 * M_PI / 50;
            float angle2 = (i+1) * 2 * M_PI / 50;
            positions.col(ctr++) << std::cos(angle1)*.5f + 0.5f, std::sin(angle1)*.5f + 0.5f, 0.f;
            positions.col(ctr++) << std::cos(angle2)*.5f + 0.5f, std::sin(angle2)*.5f + 0.5f, 0.f;
        }
        // positions.col(ctr++) << 0.5f, 0.5f, 0.f;
        // positions.col(ctr++) << -m_bRec.wi.x() * 0.5f + 0.5f, -m_bRec.wi.y() * 0.5f + 0.5f, m_bRec.wi.z() * 0.5f;
        // positions.col(ctr++) << 0.5f, 0.5f, 0.f;
        // positions.col(ctr++) << m_bRec.wi.x() * 0.5f + 0.5f, m_bRec.wi.y() * 0.5f + 0.5f, m_bRec.wi.z() * 0.5f;
        m_arrowShader->bind();
        m_arrowShader->uploadAttrib("position", positions);

        /* Update user interface */
        std::string str;
        if (m_pointCount > 1000000) {
            m_pointCountBox->setUnits("M");
            str = tfm::format("%.2f", m_pointCount * 1e-6f);
        } else if (m_pointCount > 1000) {
            m_pointCountBox->setUnits("K");
            str = tfm::format("%.2f", m_pointCount * 1e-3f);
        } else {
            m_pointCountBox->setUnits(" ");
            str = tfm::format("%i", m_pointCount);
        }
        m_pointCountBox->setValue(str);
        m_pointCountSlider->setValue((std::log((float) m_pointCount) / std::log(2.f) - 5) / 15);
    }

    ~WarpTest() {
        glDeleteTextures(2, &m_textures[0]);
        delete m_pointShader;
        delete m_gridShader;
        delete m_arrowShader;
        delete m_histogramShader;
    }

    void framebufferSizeChanged() {
        m_arcball.setSize(mSize);
    }

    bool mouseMotionEvent(const nanogui::Vector2i &p, const nanogui::Vector2i &rel,
                                  int button, int modifiers) {
        if (!Screen::mouseMotionEvent(p, rel, button, modifiers))
            m_arcball.motion(p);
        return true;
    }

    bool mouseButtonEvent(const nanogui::Vector2i &p, int button, bool down, int modifiers) {
        if (down && !m_window->visible()) {
            m_drawHistogram = false;
            m_window->setVisible(true);
            return true;
        }
        if (!Screen::mouseButtonEvent(p, button, down, modifiers)) {
            if (button == GLFW_MOUSE_BUTTON_1)
                m_arcball.button(p, down);
        }
        return true;
    }

    void drawContents() {
        /* Set up a perspective camera matrix */
        Matrix4f view, proj, model;
        view = lookAt(Eigen::Vector3f(0, 0, 2), Eigen::Vector3f(0, 0, 0), Eigen::Vector3f(0, 1, 0));
        const float viewAngle = 30, near = 0.01, far = 100;
        float fH = std::tan(viewAngle / 360.0f * M_PI) * near;
        float fW = fH * (float) mSize.x() / (float) mSize.y();
        proj = frustum(-fW, fW, -fH, fH, near, far);

        model.setIdentity();
        model = translate(nanogui::Vector3f(-0.5f, -0.5f, 0.0f));
        model = m_arcball.matrix() * model;

        if (m_drawHistogram) {
            /* Render the histograms */
            WarpType warpType = (WarpType) m_warpTypeBox->selectedIndex();
            const int spacer = 20;
            const int histWidth = (width() - 3*spacer) / 2;
            const int histHeight = (warpType == Square || warpType == Disk) ? histWidth : histWidth / 2;
            const int verticalOffset = (height() - histHeight) / 2;

            drawHistogram(Point2i(spacer, verticalOffset), nanogui::Vector2i(histWidth, histHeight), m_textures[0]);
            drawHistogram(Point2i(2*spacer + histWidth, verticalOffset), nanogui::Vector2i(histWidth, histHeight), m_textures[1]);

            auto ctx = mNVGContext;
            nvgBeginFrame(ctx, mSize[0], mSize[1], mPixelRatio);
            nvgBeginPath(ctx);
            nvgRect(ctx, spacer, verticalOffset + histHeight + spacer, width()-2*spacer, 70);
            nvgFillColor(ctx, m_testResult.first ? Color(100, 255, 100, 100) : Color(255, 100, 100, 100));
            nvgFill(ctx);
            nvgFontSize(ctx, 24.0f);
            nvgFontFace(ctx, "sans-bold");
            nvgTextAlign(ctx, NVG_ALIGN_CENTER | NVG_ALIGN_TOP);
            nvgFillColor(ctx, Color(255, 255));
            nvgText(ctx, spacer + histWidth / 2, verticalOffset - 3 * spacer,
                    "Sample histogram", nullptr);
            nvgText(ctx, 2 * spacer + (histWidth * 3) / 2, verticalOffset - 3 * spacer,
                    "Integrated density", nullptr);
            nvgStrokeColor(ctx, Color(255, 255));
            nvgStrokeWidth(ctx, 2);
            nvgBeginPath(ctx);
            nvgRect(ctx, spacer, verticalOffset, histWidth, histHeight);
            nvgRect(ctx, 2 * spacer + histWidth, verticalOffset, histWidth,
                    histHeight);
            nvgStroke(ctx);
            nvgFontSize(ctx, 20.0f);
            nvgTextAlign(ctx, NVG_ALIGN_CENTER | NVG_ALIGN_TOP);

            float bounds[4];
            nvgTextBoxBounds(ctx, 0, 0, width() - 2 * spacer,
                             m_testResult.second.c_str(), nullptr, bounds);
            nvgTextBox(
                ctx, spacer, verticalOffset + histHeight + spacer + (70 - bounds[3])/2,
                width() - 2 * spacer, m_testResult.second.c_str(), nullptr);
            nvgEndFrame(ctx);
        } else {
            /* Render the point set */
            Matrix4f mvp = proj * view * model;
            m_pointShader->bind();
            m_pointShader->setUniform("mvp", mvp);
            glPointSize(2);
            glEnable(GL_DEPTH_TEST);
            m_pointShader->drawArray(GL_POINTS, 0, m_pointCount);

            bool drawGrid = m_gridCheckBox->checked();
            if (drawGrid) {
                m_gridShader->bind();
                m_gridShader->setUniform("mvp", mvp);
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                m_gridShader->drawArray(GL_LINES, 0, m_lineCount);
                glDisable(GL_BLEND);
            }
        }
    }

    void drawHistogram(const Point2i &pos_, const nanogui::Vector2i &size_, GLuint tex) {
        Eigen::Vector2f s = -(pos_.array().cast<float>() + Eigen::Vector2f(0.25f, 0.25f).array())  / size_.array().cast<float>();
        Eigen::Vector2f e = size().array().cast<float>() / size_.array().cast<float>() + s.array();
        Matrix4f mvp = ortho(s.x(), e.x(), e.y(), s.y(), -1, 1);

        glDisable(GL_DEPTH_TEST);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex);
        m_histogramShader->bind();
        m_histogramShader->setUniform("mvp", mvp);
        m_histogramShader->setUniform("tex", 0);
        m_histogramShader->drawIndexed(GL_TRIANGLES, 0, 2);
    }

    void runTest() {
        int xres = 51, yres = 51;
        WarpType warpType = (WarpType) m_warpTypeBox->selectedIndex();

        if (warpType != Square && warpType != Disk)
            xres *= 2;

        int res = yres*xres, sampleCount = 1000 * res;
        std::unique_ptr<double[]> obsFrequencies(new double[res]);
        std::unique_ptr<double[]> expFrequencies(new double[res]);
        memset(obsFrequencies.get(), 0, res*sizeof(double));
        memset(expFrequencies.get(), 0, res*sizeof(double));

        MatrixXf points, values;
        generatePoints(sampleCount, Independent, warpType, points, values);

        for (int i=0; i<sampleCount; ++i) {
            if (values(0, i) == 0)
                continue;
            Eigen::Vector3f sample = points.col(i);
            float x, y;

            if (warpType == Square) {
                x = sample.x();
                y = sample.y();
            } else if (warpType == Disk) {
                x = sample.x() * 0.5f + 0.5f;
                y = sample.y() * 0.5f + 0.5f;
            } else {
                x = std::atan2(sample.y(), sample.x()) * INV_TWOPI;
                if (x < 0)
                    x += 1;
                y = sample.z() * 0.5f + 0.5f;
            }

            int xbin = std::min(xres-1, std::max(0, (int) std::floor(x * xres)));
            int ybin = std::min(yres-1, std::max(0, (int) std::floor(y * yres)));
            obsFrequencies[ybin * xres + xbin] += 1;
        }

        auto integrand = [&](double y, double x) -> double {
            if (warpType == Square) {
                return Warp::squareToUniformSquarePdf(Point2f(x, y));
            } else if (warpType == Disk) {
                x = x * 2 - 1; y = y * 2 - 1;
                return Warp::squareToUniformDiskPdf(Point2f(x, y));
            } else {
                x *= 2 * M_PI;
                y = y * 2 - 1;

                double sinTheta = std::sqrt(1 - y * y);
                double sinPhi = std::sin(x),
                       cosPhi = std::cos(x);

                Eigen::Vector3f v((float) (sinTheta * cosPhi),
                           (float) (sinTheta * sinPhi),
                           (float) y);

                if (warpType == UniformHemisphere)
                    return Warp::squareToUniformHemispherePdf(v);
                else if (warpType == CosineHemisphere)
                    return Warp::squareToCosineHemispherePdf(v);
                else
                    throw RTException("Invalid warp type");
            }
        };

        double scale = sampleCount;
        if (warpType == Square)
            scale *= 1;
        else if (warpType == Disk)
            scale *= 4;
        else
            scale *= 4*M_PI;

        double *ptr = expFrequencies.get();
        for (int y=0; y<yres; ++y) {
            double yStart =  y    / (double) yres;
            double yEnd   = (y+1) / (double) yres;
            for (int x=0; x<xres; ++x) {
                double xStart =  x    / (double) xres;
                double xEnd   = (x+1) / (double) xres;
                ptr[y * xres + x] = hypothesis::adaptiveSimpson2D(
                    integrand, yStart, xStart, yEnd, xEnd) * scale;
                if (ptr[y * xres + x] < 0)
                    throw RTException("The Pdf() function returned negative values!");
            }
        }

        /* Write the test input data to disk for debugging */
        hypothesis::chi2_dump(yres, xres, obsFrequencies.get(), expFrequencies.get(), "chitest.m");

        /* Perform the Chi^2 test */
        const int minExpFrequency = 5;
        const float significanceLevel = 0.01f;

        m_testResult =
            hypothesis::chi2_test(yres*xres, obsFrequencies.get(), expFrequencies.get(),
                sampleCount, minExpFrequency, significanceLevel, 1);

        float maxValue = 0, minValue = std::numeric_limits<float>::infinity();
        for (int i=0; i<res; ++i) {
            maxValue = std::max(maxValue, (float) std::max(obsFrequencies[i], expFrequencies[i]));
            minValue = std::min(minValue, (float) std::min(obsFrequencies[i], expFrequencies[i]));
        }
        minValue /= 2;
        float texScale = 1/(maxValue - minValue);

        /* Upload histograms to GPU */
        std::unique_ptr<float[]> buffer(new float[res]);
        for (int k=0; k<2; ++k) {
            for (int i=0; i<res; ++i)
                buffer[i] = ((k == 0 ? obsFrequencies[i] : expFrequencies[i]) - minValue) * texScale;

            glBindTexture(GL_TEXTURE_2D, m_textures[k]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, xres, yres,
                    0, GL_RED, GL_FLOAT, buffer.get());
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        }

        m_drawHistogram = true;
        m_window->setVisible(false);
    }

    void initializeGUI() {
        m_window = new Window(this, "Warp tester");
        m_window->setPosition(nanogui::Vector2i(15, 15));
        m_window->setLayout(new GroupLayout());

        new Label(m_window, "Input point set", "sans-bold");

        /* Create an empty panel with a horizontal layout */
        Widget *panel = new Widget(m_window);
        panel->setLayout(new BoxLayout(Orientation::Horizontal, Alignment::Middle, 0, 20));

        /* Add a slider and set defaults */
        m_pointCountSlider = new Slider(panel);
        m_pointCountSlider->setFixedWidth(55);
        m_pointCountSlider->setCallback([&](float) { refresh(); });

        /* Add a textbox and set defaults */
        m_pointCountBox = new TextBox(panel);
        m_pointCountBox->setFixedSize(nanogui::Vector2i(80, 25));

        m_pointTypeBox = new ComboBox(m_window, { "Independent", "Grid", "Stratified", "Hammersley", "Halton" });
        m_pointTypeBox->setCallback([&](int) { refresh(); });

        new Label(m_window, "Warping method", "sans-bold");
        m_warpTypeBox = new ComboBox(m_window, { "Square", "Disk", "Hemisphere (unif.)", "Hemisphere (cos)"});
        m_warpTypeBox->setCallback([&](int) { refresh(); });

        panel = new Widget(m_window);
        panel->setLayout(new BoxLayout(Orientation::Horizontal, Alignment::Middle, 0, 20));
        m_gridCheckBox = new CheckBox(m_window, "Visualize warped grid");
        m_gridCheckBox->setCallback([&](bool) { refresh(); });

        new Label(m_window,
            std::string(utf8(0x03C7).data()) +
            std::string(utf8(0x00B2).data()) + " hypothesis test",
            "sans-bold");

        Button *testBtn = new Button(m_window, "Run", ENTYPO_ICON_CHECK);
        testBtn->setBackgroundColor(Color(0, 255, 0, 25));
        testBtn->setCallback([&]{
            try {
                runTest();
            } catch (const RTException &e) {
                new MessageDialog(this, MessageDialog::Type::Warning, "Error", "An error occurred: " + std::string(e.what()));
            }
        });

        performLayout(mNVGContext);

        m_pointShader = new GLShader();
        m_pointShader->init(
            "Point shader",

            /* Vertex shader */
            "#version 330\n"
            "uniform mat4 mvp;\n"
            "in vec3 position;\n"
            "in vec3 color;\n"
            "out vec3 frag_color;\n"
            "void main() {\n"
            "    gl_Position = mvp * vec4(position, 1.0);\n"
            "    if (isnan(position.r)) /* nan (missing value) */\n"
            "        frag_color = vec3(0.0);\n"
            "    else\n"
            "        frag_color = color;\n"
            "}",

            /* Fragment shader */
            "#version 330\n"
            "in vec3 frag_color;\n"
            "out vec4 out_color;\n"
            "void main() {\n"
            "    if (frag_color == vec3(0.0))\n"
            "        discard;\n"
            "    out_color = vec4(frag_color, 1.0);\n"
            "}"
        );

        m_gridShader = new GLShader();
        m_gridShader->init(
            "Grid shader",

            /* Vertex shader */
            "#version 330\n"
            "uniform mat4 mvp;\n"
            "in vec3 position;\n"
            "void main() {\n"
            "    gl_Position = mvp * vec4(position, 1.0);\n"
            "}",

            /* Fragment shader */
            "#version 330\n"
            "out vec4 out_color;\n"
            "void main() {\n"
            "    out_color = vec4(vec3(1.0), 0.4);\n"
            "}"
        );

        m_arrowShader = new GLShader();
        m_arrowShader->init(
            "Arrow shader",

            /* Vertex shader */
            "#version 330\n"
            "uniform mat4 mvp;\n"
            "in vec3 position;\n"
            "void main() {\n"
            "    gl_Position = mvp * vec4(position, 1.0);\n"
            "}",

            /* Fragment shader */
            "#version 330\n"
            "out vec4 out_color;\n"
            "void main() {\n"
            "    out_color = vec4(vec3(1.0), 0.4);\n"
            "}"
        );

        m_histogramShader = new GLShader();
        m_histogramShader->init(
            "Histogram shader",

            /* Vertex shader */
            "#version 330\n"
            "uniform mat4 mvp;\n"
            "in vec2 position;\n"
            "out vec2 uv;\n"
            "void main() {\n"
            "    gl_Position = mvp * vec4(position, 0.0, 1.0);\n"
            "    uv = position;\n"
            "}",

            /* Fragment shader */
            "#version 330\n"
            "out vec4 out_color;\n"
            "uniform sampler2D tex;\n"
            "in vec2 uv;\n"
            "/* http://paulbourke.net/texture_colour/colourspace/ */\n"
            "vec3 colormap(float v, float vmin, float vmax) {\n"
            "    vec3 c = vec3(1.0);\n"
            "    if (v < vmin)\n"
            "        v = vmin;\n"
            "    if (v > vmax)\n"
            "        v = vmax;\n"
            "    float dv = vmax - vmin;\n"
            "    \n"
            "    if (v < (vmin + 0.25 * dv)) {\n"
            "        c.r = 0.0;\n"
            "        c.g = 4.0 * (v - vmin) / dv;\n"
            "    } else if (v < (vmin + 0.5 * dv)) {\n"
            "        c.r = 0.0;\n"
            "        c.b = 1.0 + 4.0 * (vmin + 0.25 * dv - v) / dv;\n"
            "    } else if (v < (vmin + 0.75 * dv)) {\n"
            "        c.r = 4.0 * (v - vmin - 0.5 * dv) / dv;\n"
            "        c.b = 0.0;\n"
            "    } else {\n"
            "        c.g = 1.0 + 4.0 * (vmin + 0.75 * dv - v) / dv;\n"
            "        c.b = 0.0;\n"
            "    }\n"
            "    return c;\n"
            "}\n"
            "void main() {\n"
            "    float value = texture(tex, uv).r;\n"
            "    out_color = vec4(colormap(value, 0.0, 1.0), 1.0);\n"
            "}"
        );

        /* Upload a single quad */
        MatrixXf positions(2, 4);
        MatrixXu indices(3, 2);
        positions <<
            0, 1, 1, 0,
            0, 0, 1, 1;
        indices <<
            0, 2,
            1, 3,
            2, 0;
        m_histogramShader->bind();
        m_histogramShader->uploadAttrib("position", positions);
        m_histogramShader->uploadIndices(indices);

        glGenTextures(2, &m_textures[0]);
        glBindTexture(GL_TEXTURE_2D, m_textures[0]);

        mBackground.setZero();
        drawAll();

        /* Set default and register slider callback */
        m_pointCountSlider->setValue(7.f/15.f);

        refresh();
        setVisible(true);
        framebufferSizeChanged();
    }
private:
    nanogui::GLShader *m_pointShader = nullptr;
    nanogui::GLShader *m_gridShader = nullptr;
    nanogui::GLShader *m_histogramShader = nullptr;
    nanogui::GLShader *m_arrowShader = nullptr;
    nanogui::Window *m_window;
    nanogui::Slider *m_pointCountSlider;
    nanogui::TextBox *m_pointCountBox;
    GLuint m_textures[2];
    nanogui::ComboBox *m_pointTypeBox;
    nanogui::ComboBox *m_warpTypeBox;
    nanogui::CheckBox *m_gridCheckBox;
    nanogui::Arcball m_arcball;
    int m_pointCount, m_lineCount;
    bool m_drawHistogram;
    std::pair<bool, std::string> m_testResult;

    std::random_device rd;
    std::default_random_engine gen;
    std::uniform_real_distribution<float> dist;
};

int main(int argc, char **argv) {
    init();

    WarpTest *screen = new WarpTest();
    mainloop();
    delete screen;

    shutdown();

    return 0;
}
