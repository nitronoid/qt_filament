#ifndef FILAMENT_RAII
#define FILAMENT_RAII

#include <memory>
#include <utils/Entity.h>
#include <filament/FilamentAPI.h>
#include <filament/Engine.h>

// Deleter designed to be used in tandem with a unique_ptr to destroy filament
// entities using the provided filament engine
template <typename T>
struct FilamentEngineDeleter
{
  std::shared_ptr<filament::Engine> m_engine = nullptr;

  inline void operator()(T* io_ptr) noexcept
  {
    static_assert(
      std::is_base_of<filament::FilamentAPI, T>::value,
      "Cannot use the filament engine deleter, for non-filament types");
    if (io_ptr && m_engine)
    {
      m_engine->destroy(io_ptr);
    }
  }
};
// Unique pointer designed to store a filament API objects
template <typename T>
using FilamentScopedPointer = std::unique_ptr<T, FilamentEngineDeleter<T>>;

// Class designed using RAII to ensure entities are properly destroyed
class FilamentScopedEntity
{
public:
  // Default construct in same way as an Entity can be
  FilamentScopedEntity() noexcept = default;
  // Copying is disallowed to prevent double destroying
  FilamentScopedEntity(const FilamentScopedEntity&) = delete;
  FilamentScopedEntity& operator=(const FilamentScopedEntity&) = delete;
  // Standard move semantics
  FilamentScopedEntity(FilamentScopedEntity&&) noexcept = default;
  FilamentScopedEntity& operator=(FilamentScopedEntity&&) noexcept = default;

  // Construct from an engine pointer and entity
  FilamentScopedEntity(utils::Entity&& i_entity,
                       std::shared_ptr<filament::Engine> i_engine) noexcept;

  // Construct from an engine
  FilamentScopedEntity(std::shared_ptr<filament::Engine> i_engine) noexcept;

  // Assign from an entity
  FilamentScopedEntity& operator=(utils::Entity&& i_entity) noexcept;

  ~FilamentScopedEntity();

  // Implicitly cast to an entity in const context
  operator utils::Entity() const noexcept;

  // Implicitly cast to a reference to an entity in non-const context
  operator utils::Entity&() noexcept;

  // Assign from an entity
  void set_entity(utils::Entity&& i_entity) noexcept;

  // Assign from an engine
  void set_engine(std::shared_ptr<filament::Engine> i_engine) noexcept;

private:
  utils::Entity m_entity;
  std::shared_ptr<filament::Engine> m_engine = nullptr;
};

#endif  // FILAMENT_RAII
