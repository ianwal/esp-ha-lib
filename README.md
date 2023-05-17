# esp-ha-lib
WIP Library for interfacing with Home Assistants REST API

None of the functions are guaranteed stable.


Entity:

POST/GET using get_entity() and post_entity() using HAEntity struct

To POST an entity, the HAEntity req must have at least the entity_id. It should have the state as well since that is the main entity data value. 

To GET an entity, only the entity name is needed. For example, to get an entity named sensor.mysensor, just call get_entity("sensor.mysensor") and the corresponding HAEntity will be returned with entity data (or NULL if it fails). This must be manually freed using HAEntity_destroy().

Attributes can be added as a key:value pair using add_entity_attribute()
