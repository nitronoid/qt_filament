#include <QApplication>
#include <QSurfaceFormat>
#include "app_window.h"
#include "native_window_widget.h"
#include "filament_raii.h"
#include <random>
#include <filament/Material.h>
#include <filament/MaterialInstance.h>
#include <filament/RenderableManager.h>
#include <filament/Scene.h>
#include <filament/View.h>
#include <filament/TransformManager.h>
#include <filament/VertexBuffer.h>
#include <filament/IndexBuffer.h>

// To reduce the verbosity of filament code
namespace fl = filament;
namespace flm = filament::math;
namespace flut = utils;

// Structure to hold our vertex attributes
struct Vertex
{
  flm::float2 position;
  uint32_t color;
};
static_assert(sizeof(Vertex) == 12, "sizeof(Vertex) != 12");

// Triangle corners
static const Vertex TRIANGLE_VERTICES[3] = {
  {{1, 0}, 0xffff0000u},
  {{cos(M_PI * 2 / 3), sin(M_PI * 2 / 3)}, 0xff00ff00u},
  {{cos(M_PI * 4 / 3), sin(M_PI * 4 / 3)}, 0xff0000ffu},
};
static constexpr uint16_t TRIANGLE_INDICES[3] = {0, 1, 2};

// This needs to be generated from the sample bakedColor.mat
// $>  matc -o bakedColor.inc -f header bakedColor.mat
static constexpr uint8_t BAKED_COLOR_PACKAGE[] = {
#include "materials/bakedColor.inc"
};

// Our filament rendering window
class FilamentWindowWidget final : public NativeWindowWidget
{
public:
  explicit FilamentWindowWidget(QWidget* i_parent,
                                const filament::Engine::Backend i_backend)
    : NativeWindowWidget(i_parent)
    , m_engine(filament::Engine::create(i_backend),
               [](filament::Engine* i_engine) { i_engine->destroy(&i_engine); })
    , m_swap_chain(nullptr, {m_engine})
    , m_renderer(m_engine->createRenderer(), {m_engine})
    , m_camera(m_engine->createCamera(), {m_engine})
    , m_view(m_engine->createView(), {m_engine})
    , m_scene(m_engine->createScene(), {m_engine})
    , m_vertex_buffer(nullptr, {m_engine})
    , m_index_buffer(nullptr, {m_engine})
    , m_material(nullptr, {m_engine})
    , m_triangle(m_engine)
  {
  }

  virtual void init_impl(void* io_native_window) override
  {
    NativeWindowWidget::init_impl(io_native_window);
    // Create our swap chain for displaying rendered frames
    m_swap_chain.reset(m_engine->createSwapChain(io_native_window));

    // Link the camera and scene to our view point
    m_view->setCamera(m_camera.get());
    m_view->setScene(m_scene.get());

    // Set up the render view point
    setup_camera();

    // Screen space effects
    m_view->setClearColor({0.1f, 0.125f, 0.25f, 1.0f});
    m_view->setPostProcessingEnabled(false);
    m_view->setDepthPrepass(fl::View::DepthPrepass::DISABLED);

    // To reduce the verbosity of set-up code
    using flvb = fl::VertexBuffer;
    using flva = fl::VertexAttribute;
    // Build the vertex buffer, we only have position and color attributes
    m_vertex_buffer.reset(
      flvb::Builder()
        .vertexCount(3)
        .bufferCount(1)
        .attribute(flva::POSITION, 0, flvb::AttributeType::FLOAT2, 0, 12)
        .attribute(flva::COLOR, 0, flvb::AttributeType::UBYTE4, 8, 12)
        .normalized(flva::COLOR)
        .build(*m_engine));
    // Write our vertex data into the buffer
    auto vbuf_data = flvb::BufferDescriptor(TRIANGLE_VERTICES, 36, nullptr);
    m_vertex_buffer->setBufferAt(*m_engine, 0, std::move(vbuf_data));

    // To reduce the verbosity of set-up code
    using flib = fl::IndexBuffer;
    // Build the index buffer
    m_index_buffer.reset(flib::Builder()
                           .indexCount(3)
                           .bufferType(fl::IndexBuffer::IndexType::USHORT)
                           .build(*m_engine));
    // Write our index data into the buffer
    auto ibuf_data = flib::BufferDescriptor(TRIANGLE_INDICES, 6, nullptr);
    m_index_buffer->setBuffer(*m_engine, std::move(ibuf_data));

    // Build a material from our generated header
    m_material.reset(
      fl::Material::Builder()
        .package((void*)BAKED_COLOR_PACKAGE, sizeof(BAKED_COLOR_PACKAGE))
        .build(*m_engine));

    // We want to render triangles
    const auto prim_type = fl::RenderableManager::PrimitiveType::TRIANGLES;
    // Build a renderable entity that will be our triangle, from the buffers
    // and the material
    m_triangle = flut::EntityManager::get().create();
    fl::RenderableManager::Builder(1)
      .boundingBox({{-1.f, -1.f, -1.f}, {1.f, 1.f, 1.f}})
      .material(0, m_material->getDefaultInstance())
      .geometry(0, prim_type, m_vertex_buffer.get(), m_index_buffer.get(), 0, 3)
      .culling(false)
      .receiveShadows(false)
      .castShadows(false)
      .build(*m_engine, m_triangle);
    m_scene->addEntity(m_triangle);
  }

private:
  void setup_camera()
  {
    // Get the width and height of our window, scaled by the pixel ratio
    const auto pixel_ratio = devicePixelRatio();
    const uint32_t w = static_cast<uint32_t>(width() * pixel_ratio);
    const uint32_t h = static_cast<uint32_t>(height() * pixel_ratio);

    // Set our view-port size
    m_view->setViewport({0, 0, w, h});

    // setup view matrix
    const flm::float3 eye(0.f, 0.f, 1.f);
    const flm::float3 target(0.f);
    const flm::float3 up(0.f, 1.f, 0.f);
    m_camera->lookAt(eye, target, up);

    // setup projection matrix
    constexpr float k_zoom = 1.5f;
    const float aspect = float(w) / h;
    m_camera->setProjection(fl::Camera::Projection::ORTHO,
                            -aspect * k_zoom,
                            aspect * k_zoom,
                            -k_zoom,
                            k_zoom,
                            0,
                            1);
  }

  virtual void resize_impl() override
  {
    // Don't attempt to access any filament entities before we've initialized
    if (!m_is_init)
      return;
    NativeWindowWidget::resize_impl();
    // Recalculate our camera matrices
    setup_camera();
  }

  virtual void draw_impl() override
  {
    NativeWindowWidget::draw_impl();
    // beginFrame() returns false if we need to skip a frame
    if (m_renderer->beginFrame(m_swap_chain.get()))
    {
      m_renderer->render(m_view.get());
      m_renderer->endFrame();
    }
  }

  virtual void closeEvent(QCloseEvent* i_event) override
  {
    QWidget::closeEvent(i_event);
    // We need to ensure all rendering operations have completed before we
    // destroy our engine registered objects.
    // Safe to assume we won't be issuing anymore render calls after this
    filament::Fence::waitAndDestroy(m_engine->createFence());
  }

private:
  // Store a shared pointer to the engine, all of our entities will also store
  std::shared_ptr<filament::Engine> m_engine;

  // Scoped unique pointers to all engine registered objects
  FilamentScopedPointer<filament::SwapChain> m_swap_chain;
  FilamentScopedPointer<filament::Renderer> m_renderer;
  FilamentScopedPointer<filament::Camera> m_camera;
  FilamentScopedPointer<filament::View> m_view;
  FilamentScopedPointer<filament::Scene> m_scene;
  FilamentScopedPointer<filament::VertexBuffer> m_vertex_buffer;
  FilamentScopedPointer<filament::IndexBuffer> m_index_buffer;
  FilamentScopedPointer<filament::Material> m_material;

  // Scoped entity for our renderable
  FilamentScopedEntity m_triangle;
};

int main(int argc, char* argv[])
{
  // Create the application
  QApplication app(argc, argv);
  // Create a new main window
  AppWindow window;
  // Create our filament window
  auto filament_widget = std::make_shared<FilamentWindowWidget>(
    &window, filament::Engine::Backend::OPENGL);
  // Initialize the filament entities and set-up cameras
  filament_widget->init();
  // Initialize the main window using our filament scene
  window.init(filament_widget);
  // Show it
  window.show();
  // Hand control over to Qt framework
  return app.exec();
}

