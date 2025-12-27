//
//  Object.hpp
//
//  Created by Roald Christesen on 17.11.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 10.07.2025
//

#ifndef GrainObject_hpp
#define GrainObject_hpp

#include "Grain.hpp"


namespace Grain {

    class String;


    /**
     *  @brief Base class providing a generic interface for parameter setting and
     *         message handling.
     *
     *  This abstract base class defines a common interface for derived objects that
     *  need runtime identification, parameter setting, and message handling. It can
     *  be extended to implement custom behavior for specific object types.
     */
    class BaseObject {
    public:
        BaseObject() = default;
        virtual ~BaseObject() = default;

        [[nodiscard]] virtual const char* className() const noexcept;
        [[nodiscard]] virtual bool isClass(const char* name) const noexcept;

        virtual ErrorCode setParam(const String& name, const String& value) noexcept;
        virtual ErrorCode handleMessage(const char* message, const char* value) noexcept;
    };


    /**
     *  @brief Reference-counted object that extends BaseObject.
     *
     *  This class implements a simple manual reference counting mechanism
     *  to manage the lifetime of objects. It provides static methods for
     *  retaining and releasing references, and overrides the class name
     *  identification method from BaseObject.
     */
    class Object : public BaseObject {
    protected:
        int64_t m_retain_counter = 0;

    public:
        Object();
        ~Object() override;

        [[nodiscard]] const char* className() const noexcept override;

        [[nodiscard]] int64_t retainCounter() const noexcept;
        static void retain(Object* object);
        static Object* release(Object* object);
    };


} // End of namespace Grain

#endif // GrainObject_hpp
