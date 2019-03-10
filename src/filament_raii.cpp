#include "filament_raii.h"

// Construct from an engine pointer and entity
FilamentScopedEntity::FilamentScopedEntity(
  utils::Entity&& i_entity, std::shared_ptr<filament::Engine> i_engine) noexcept
  : m_entity(i_entity), m_engine(std::move(i_engine))
{
}

// Construct from an engine
FilamentScopedEntity::FilamentScopedEntity(
  std::shared_ptr<filament::Engine> i_engine) noexcept
  : m_engine(std::move(i_engine))
{
}

// Assign from an entity
FilamentScopedEntity& FilamentScopedEntity::
operator=(utils::Entity&& i_entity) noexcept
{
  m_entity = i_entity;
  return *this;
}

FilamentScopedEntity::~FilamentScopedEntity()
{
  // Only destroy if we have a valid entity and engine
  if (m_engine && m_entity)
  {
    m_engine->destroy(m_entity);
  }
}

// Implicitly cast to an entity in const context
FilamentScopedEntity::operator utils::Entity() const noexcept
{
  return m_entity;
}

// Implicitly cast to a reference to an entity in non-const context
FilamentScopedEntity::operator utils::Entity&() noexcept
{
  return m_entity;
}

// Assign from an entity
void FilamentScopedEntity::set_entity(utils::Entity&& i_entity) noexcept
{
  m_entity = i_entity;
}

// Assign from an engine
void FilamentScopedEntity::set_engine(
  std::shared_ptr<filament::Engine> i_engine) noexcept
{
  m_engine = std::move(i_engine);
}
