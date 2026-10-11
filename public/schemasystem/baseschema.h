#ifndef BASESCHEMA_H
#define BASESCHEMA_H

#ifdef _WIN32
#pragma once
#endif

enum SchemaBuiltinType_t
{
	SCHEMA_BUILTIN_TYPE_INVALID = 0,
	SCHEMA_BUILTIN_TYPE_VOID,
	SCHEMA_BUILTIN_TYPE_CHAR,
	SCHEMA_BUILTIN_TYPE_INT8,
	SCHEMA_BUILTIN_TYPE_UINT8,
	SCHEMA_BUILTIN_TYPE_INT16,
	SCHEMA_BUILTIN_TYPE_UINT16,
	SCHEMA_BUILTIN_TYPE_INT32,
	SCHEMA_BUILTIN_TYPE_UINT32,
	SCHEMA_BUILTIN_TYPE_INT64,
	SCHEMA_BUILTIN_TYPE_UINT64,
	SCHEMA_BUILTIN_TYPE_FLOAT32,
	SCHEMA_BUILTIN_TYPE_FLOAT64,
	SCHEMA_BUILTIN_TYPE_BOOL,
	SCHEMA_BUILTIN_TYPE_COUNT,
};

// Works in combination with SchemaCollectionManipulatorFn_t
enum SchemaCollectionManipulatorAction_t
{
	// Returns count of the collection, index1 & index2 is unused
	SCHEMA_COLLECTION_MANIPULATOR_ACTION_GET_COUNT = 0,

	// Returns element from the collection at index1, index2 is unused
	SCHEMA_COLLECTION_MANIPULATOR_ACTION_GET_ELEMENT_CONST,
	SCHEMA_COLLECTION_MANIPULATOR_ACTION_GET_ELEMENT,

	// Swaps elements in a collection, index1 & index2 is first and second elements to swap
	SCHEMA_COLLECTION_MANIPULATOR_ACTION_SWAP_ELEMENTS,
	// Inserts elements to a collection at index1 where index2 is how much elements to insert
	SCHEMA_COLLECTION_MANIPULATOR_ACTION_INSERT_BEFORE,
	// Removes elements from a collection at index1 where index2 is how much elements to remove
	SCHEMA_COLLECTION_MANIPULATOR_ACTION_REMOVE_MULTIPLE,

	// Sets the count of a collection of size index1, index2 is unused
	SCHEMA_COLLECTION_MANIPULATOR_ACTION_SET_COUNT,
};

typedef void *(*SchemaCollectionManipulatorFn_t)(SchemaCollectionManipulatorAction_t eAction, void *pCollection, int index1, int index2);

#endif // BASESCHEMA_H
