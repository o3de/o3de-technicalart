import azlmbr
import azlmbr.bus as bus
import azlmbr.entity as entity
import azlmbr.legacy.general as general
import azlmbr.editor as editor
import azlmbr.math as math
import azlmbr.asset as asset
import azlmbr.paths as projectPath


def print_property_list(component_id):
    """
    This function will get the component property list, this is mostly used for development

    Args:
        component_id (Object): This is the Entity Component Object ID

    Returns:
        value(String): value of property list for component
    """
    # Print the property list of a component
    property_list = editor.EditorComponentAPIBus(bus.Broadcast, 'BuildComponentPropertyList', component_id)

def has_component(entity_id, type_id_list):
    """
    This function will get the component property list, this is mostly used for development

    Args:
        entity_id (Object): This is the Entity Object ID
        type_id_list (Object List): This is the list of component type ids

    Returns:
        has_component (bool): bool if component
    """
    component_bool = editor.EditorComponentAPIBus(bus.Broadcast, 'HasComponentOfType', entity_id, type_id_list[0])
    return component_bool

def add_component(entity_id, component_name):
    """
    This function will get the component property list, this is mostly used for development

    Args:
        entity_id (Object): This is the Entity Object ID
        type_id (Object List): component type ids
    """
    type_id = editor.EditorComponentAPIBus(bus.Broadcast, 'FindComponentTypeIdsByEntityType', [component_name], 0)
    component_type_id = type_id[0]
    editor.EditorComponentAPIBus(bus.Broadcast, 'AddComponentsOfType', entity_id, [component_type_id])

def get_property(component_id, property_path):
    """
    This function will get the component property

    Args:
        component_id (Object): This is the Entity Component Object ID
        component property_path (Path): This is the Compoent Property Path you wish to set a value.

    Returns:
        value(String): value of property
    """
    # you can now get or set one of those properties by their strings, such as:
    object_property = editor.EditorComponentAPIBus(bus.Broadcast, 'GetComponentProperty', component_id, property_path)
    
    if(object_property.IsSuccess()):
        value = object_property.GetValue()
        return value
    else:
        return None

def set_property(component_id, property_path):
    """
    This function will set the component property

    Args:
        component_id (Object): This is the Entity Component Object ID
        component property_path (Path): This is the Compoent Property Path you wish to set a value.
    """
    # you can now get or set one of those properties by their strings, such as:
    editor.EditorComponentAPIBus(bus.Broadcast, 'SetComponentProperty', component_id, property_path, 200)
    new_obj = editor.EditorComponentAPIBus(bus.Broadcast, 'GetComponentProperty', component_id, property_path)
    # how to get a float value and print it to the log
    if(new_obj.IsSuccess()):
        new_value = new_obj.GetValue()
        float_value = new_value.GetValue()

def get_name_from_id(entity_id):
    """
    This function will get a name from and entity id.

    Args:
        child_id (Object): This is the Childs Entity ID

    Returns:
        (String): Name of Entity

    """
    name = editor.EditorEntityInfoRequestBus(bus.Event, 'GetName', entity_id)
    return name

def create_object_from_id(id_int):
    """
    This function will create an object from an id number

    Args:
        id_int (int): This is the id number

    Returns:
        (Object): the object

    """
    object_id = entity.EntityId(id_int)
    return object_id

def get_parent(child_id):
    """
    This function will return from the given ID the entity parent

    Args:
        child_id (Object): This is the Childs Entity ID

    Returns:
        parent_id (Object): Object ID of Parent Entity
        parent_name (String): Name of Parent Entity string

    """
    parent_id = editor.EditorEntityInfoRequestBus(bus.Event, 'GetParent', child_id)
    parent_name = get_name_from_id(parent_id)
    
    return parent_id, parent_name

def search_for_entity(entity_name):
    """
    This function will search for an entity

    Args:
        entity_name (sting): Name string: search_filter.names = ['TestEntity']

    Returns:
        (Object): entity_object_id

    """
    search_filter = azlmbr.entity.SearchFilter()
    search_filter.names = [entity_name]
    entity_object_id = azlmbr.entity.SearchBus(bus.Broadcast, 'SearchEntities', search_filter)
    return entity_object_id

def convert_entity_object_to_id(entity_object_id):
    """
    This function will create an object to an id int

    Args:
        entity_object_id (Object): <EntityComponentIdPair via PythonProxyObject at 2428988078368>

    Returns:
        (int): entity_id INT

    """
    entity_id = entity_object_id[0]
    return entity_id

def find_component_by_type_id(entity_id, component_name):
    """
    This function will find a component by type_id

    Args:
        entity_id (int): Example [4817570267466324385]
        component_name (string): Example ['PhysX Hinge Joint']

    Returns:
        (any): component_object_value

    """
    GAME = azlmbr.entity.EntityType().Game
    type_ids = editor.EditorComponentAPIBus(bus.Broadcast, "FindComponentTypeIdsByEntityType", component_name, GAME)
    component_type_id = type_ids[0]
    component_outcome = editor.EditorComponentAPIBus(bus.Broadcast, "GetComponentOfType", entity_id, component_type_id)
    component_object_value = component_outcome.GetValue()
    return component_object_value

def get_value_from_property_path(component_object_id, property_path):
    """
    This function will get a component value from a property path

    Args:
        component_object_id (Object): Example <EntityComponentIdPair via PythonProxyObject at 2428988078368>
        property_path (string): Example "Standard Joint Parameters|Lead Entity"

    Returns:
        (int): entity_id Example [14241452422305056972]

    """
    entity_outcome = editor.EditorComponentAPIBus(bus.Broadcast, "GetComponentProperty", component_object_id, property_path)
    entity_id = entity_outcome.GetValue()
    return entity_id

def get_transforms_from_id(entity_id):
    """
    This function will get a transforms from and entity id.

    Args:
        entity_id (Object): This is the Entity ID

    Returns:
        local_translation (Vector3): Entity Translation Vector 3 (x,y,x)
        local_rotation (Vector3): Entity Rotation Vector 3 (x,y,x)
        local_uniform_scale (Vector3): Entity Uniform Scale Vector 3 (x,y,x)

    """
    local_translation = azlmbr.components.TransformBus(azlmbr.bus.Event, 'GetLocalTranslation', entity_id)
    local_rotation = azlmbr.components.TransformBus(azlmbr.bus.Event, 'GetLocalRotation', entity_id)
    local_uniform_scale = azlmbr.components.TransformBus(azlmbr.bus.Event, 'GetLocalUniformScale', entity_id) # Maybe root we can get GetWorldUniformScale

    return local_translation, local_rotation, local_uniform_scale

def get_component_type_id(entity_id):
    """
    This function will get a name from and entity id.
    We will have to check one at a time as some API only works at run time and not in editor.

    Args:
        entity_id (Object): This is the Entity ID
    
    Returns:
        components_on_entity (List): List of Components on an Entity by Name Type String
        components_on_entity_id (List): List of component IDs on an Entity

    """
    components_on_entity = []
    components_on_entity_id = []

    type_ids_list = editor.EditorComponentAPIBus(bus.Broadcast, 'FindComponentTypeIdsByEntityType', ["Mesh",
                                                                "Actor", "Material", "PhysX Collider",
                                                                "PhysX Rigid Body", "PhysX Ball Joint",
                                                                "PhysX Hinge Joint", "PhysX Fixed Joint",
                                                                "PhysX Prismatic Joint"], 0)
    
    mesh_component_type_id = type_ids_list[0]
    actor_component_type_id = type_ids_list[1]
    material_component_type_id = type_ids_list[2]
    collider_component_type_id = type_ids_list[3]
    rigid_body_component_type_id = type_ids_list[4]
    ball_joint_component_type_id = type_ids_list[5]
    hinge_joint_component_type_id = type_ids_list[6]
    fixed_joint_component_type_id = type_ids_list[7]
    prismatic_joint_component_type_id = type_ids_list[8]

    #print(f'This is the list of Type IDs >: {type_ids_list}')

    # If entity has component
    has_mesh_component = editor.EditorComponentAPIBus(bus.Broadcast, 'HasComponentOfType', entity_id, mesh_component_type_id)
    has_material_component = editor.EditorComponentAPIBus(bus.Broadcast, 'HasComponentOfType', entity_id, material_component_type_id)
    has_collider_component = editor.EditorComponentAPIBus(bus.Broadcast, 'HasComponentOfType', entity_id, collider_component_type_id)
    has_rigid_body_component = editor.EditorComponentAPIBus(bus.Broadcast, 'HasComponentOfType', entity_id, rigid_body_component_type_id)
    has_ball_joint_component = editor.EditorComponentAPIBus(bus.Broadcast, 'HasComponentOfType', entity_id, ball_joint_component_type_id)
    has_hinge_joint_component = editor.EditorComponentAPIBus(bus.Broadcast, 'HasComponentOfType', entity_id, hinge_joint_component_type_id)
    has_fixed_joint_component = editor.EditorComponentAPIBus(bus.Broadcast, 'HasComponentOfType', entity_id, fixed_joint_component_type_id)
    has_prismatic_joint_component = editor.EditorComponentAPIBus(bus.Broadcast, 'HasComponentOfType', entity_id, prismatic_joint_component_type_id)

    if has_mesh_component:
        components_on_entity.append('mesh')
        components_on_entity_id.append(mesh_component_type_id)
    if has_material_component:
        components_on_entity.append('material')
    components_on_entity_id.append(material_component_type_id)
    if has_collider_component:
        components_on_entity.append('collision')
        components_on_entity_id.append(collider_component_type_id)
    if has_rigid_body_component:
        components_on_entity.append('rigid_body')
        components_on_entity_id.append(rigid_body_component_type_id)
    if has_ball_joint_component:
        components_on_entity.append('continuous') # Ball joint is Continuous Joint in URDF
        components_on_entity_id.append(ball_joint_component_type_id)
    if has_hinge_joint_component:
        components_on_entity.append('revolute') # Hinge joint is Revolute Joint in URDF
        components_on_entity_id.append(hinge_joint_component_type_id)
    if has_fixed_joint_component:
        components_on_entity.append('fixed') # Fixed joint is FixedJoint in URDF
        components_on_entity_id.append(fixed_joint_component_type_id)
    if has_prismatic_joint_component:
        components_on_entity.append('prismatic') # Prismatic joint is Prismatic Joint in URDF
        components_on_entity_id.append(prismatic_joint_component_type_id)

    return components_on_entity, components_on_entity_id