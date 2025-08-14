//
//  Random.hpp
//
//  Created by Roald Christesen on 06.05.2014
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 11.07.2025
//

#ifndef GrainRandom_hpp
#define GrainRandom_hpp

#include <random>
#include <cmath>
#include <chrono>


namespace Grain {

    /**
     *  @class Random
     *  @brief Utility class providing static methods for random number generation.
     *
     *  This class wraps platform-specific random number generation (`arc4random`)
     *  to provide a suite of useful random utilities for floats, integers, and
     *  booleans.
     *
     *  The methods are all `static`, so no instance of `Random` is needed.
     */
    class Random {
    public:
        /**
         *  @brief Generates a random floating-point number in [0, 1].
         *  @return Random float in range [0, 1].
         */
        [[nodiscard]] static float next() noexcept {
            return static_cast<float>(arc4random()) * kUInt32Reciprocal;
        }

        /**
         *  @brief Generates a random floating-point number in [0, max].
         *  @param max Upper bound for the random value.
         *  @return Random float in range [0, max].
         */
        [[nodiscard]] static float next(float max) noexcept {
            return static_cast<float>(arc4random()) * kUInt32Reciprocal * max;
        }

        /**
         *  @brief Generates a random floating-point number in [min, max].
         *  @param min Lower bound.
         *  @param max Upper bound.
         *  @return Random float in range [min, max].
         */
        [[nodiscard]] static float next(float min, float max) noexcept {
            return (static_cast<float>(arc4random()) * kUInt32Reciprocal) * (max - min) + min;
        }

        /**
         *  @brief Generates a random number in [-1.0, 1.0].
         *  @return Random float in range [-1.0, 1.0].
         */
        [[nodiscard]] static float nextBipolar() noexcept {
            return static_cast<float>(arc4random()) * kUint32Reciprocal2 - 1.0f;
        }

        /**
         *  @brief Generates a random number in [-max, max].
         *  @param max Maximum absolute value.
         *  @return Random float in range [-max, max].
         */
        [[nodiscard]] static float nextBipolar(float max) noexcept {
            return (static_cast<float>(arc4random()) * kUint32Reciprocal2 - 1.0f) * max;
        }

        /**
         *  @brief Returns a random integer in [0, std::numeric_limits<int32_t>::max()].
         *  @return Random 32-bit signed integer.
         */
        [[nodiscard]] static int32_t nextInt() noexcept { return nextInt(0, std::numeric_limits<int32_t>::max()); }

        /**
         *  @brief Returns a random integer in [0, max].
         *  @param max Maximum value (inclusive).
         *  @return Random 32-bit signed integer.
         */
        [[nodiscard]] static int32_t nextInt(int32_t max) noexcept { return nextInt(0, max); }

        [[nodiscard]] static int32_t nextInt(int32_t min, int32_t max) noexcept;

        /**
         *  @brief Returns a random unsigned byte [0, 255].
         *  @return Random 8-bit unsigned integer.
         */
        [[nodiscard]] static uint8_t nextUByte() noexcept { return static_cast<uint8_t >(nextInt(0, 255)); }

        [[nodiscard]] static char nextChar(const char* table, int32_t table_length) noexcept;

        /**
         *  @brief Returns true with 50% probability.
         *  @return Boolean result of random chance.
         */
        [[nodiscard]] static bool chance() noexcept { return next() > 0.5f; }

        /**
         *  @brief Returns true with the given probability threshold.
         *  @param threshold Probability value in [0, 1].
         *  @return True with probability < threshold.
         */
        [[nodiscard]] static bool chance(float threshold) noexcept { return next() < threshold; }

        /**
         * @brief Returns a white noise sample in [-1, 1].
         * @return Random noise sample.
         */

    public:
        static constexpr float kUInt32Reciprocal = 1.0f / static_cast<float>(std::numeric_limits<uint32_t>::max());
        static constexpr float kUint32Reciprocal2 = 2.0f / static_cast<float>(std::numeric_limits<uint32_t>::max());
    };


    /**
     *  @class RandomArray
     *  @brief Utility class for selecting random elements from a static array.
     *
     *  This templated class provides functionality to randomly select elements
     *  from a fixed-size array passed at construction. It assumes the array is
     *  valid for the specified size and does not perform deep copies.
     *
     *  @tparam T Type of the array elements.
     *
     *  @note The pointer passed to the constructor must remain valid for the entire
     *        lifetime of the RandomArray object. The class does not take ownership
     *        of the array memory and does not perform any bounds or lifetime checks.
     */
    template <class T>
    class RandomArray {

    public:
        /**
         *  @brief Constructs a RandomArray with the given array and size.
         *  @param array Pointer to the array.
         *  @param size Number of elements in the array.
         */
        RandomArray(T* array, int32_t size) {
            m_array = array;
            m_array_size = size;
        }

        /**
         *  @brief Checks if the RandomArray has been correctly initialized.
         *  @return `true` if the array pointer is non-null and size is greater than 0.
         */
        [[nodiscard]] bool isValid() const noexcept { return m_array != nullptr && m_array_size > 0; }

        /**
         *  @brief Returns a random element from the array.
         *  @return A randomly selected element of type T.
         *  @note Assumes that the array is properly initialized and the index is valid.
         */
        [[nodiscard]] T next() const noexcept {
            int32_t index = Random::nextInt(m_array_size - 1);
            if (index >= m_array_size) {
                index = m_array_size - 1;
            }
            return m_array[index];
        }

    protected:
        T* m_array = nullptr;
        int32_t m_array_size = 0;
    };


    /**
     *  @class IntRand
     *  @brief Utility class for generating random integers.
     *
     *  This class uses a Mersenne Twister (`std::mt19937`) engine to generate
     *  uniformly distributed random integers in a given range.
     */
    class IntRand {
    public:
        IntRand() noexcept { setup(0, 1000, true); }
        IntRand(int32_t min, int32_t max) noexcept { setup(min, max, true); }

        [[nodiscard]] int32_t min() const noexcept { return m_min; }
        [[nodiscard]] int32_t max() const noexcept { return m_max; }
        void setup(int32_t min, int32_t max, bool seed_flag) noexcept;
        [[nodiscard]] int32_t nextInt() { return m_distribution(m_generator); }

    protected:
        std::mt19937 m_generator;
        std::uniform_int_distribution<int32_t> m_distribution;
        int32_t m_min = 0;
        int32_t m_max = 1000;
    };


    /**
     *  @brief Abstract base class for real number random generators.
     *
     *  Provides common infrastructure such as seeding and generator setup.
     *  Derived classes must define their own distribution and override `next()`.
     */
    class BaseRand {
    public:
        BaseRand(bool seed_flag = true) noexcept {
            if (seed_flag) {
                auto seed = std::chrono::system_clock::now().time_since_epoch().count();
                m_generator.seed(static_cast<unsigned int>(seed));
            }
        }

        virtual ~BaseRand() = default;

        /**
         * @brief Generate the next random number.
         * @return The next random value.
         */
        virtual float next() noexcept = 0;

    protected:
        std::mt19937 m_generator;
    };


    /**
     *  @class RealRand
     *  @brief Uniform distribution random number generator for floats.
     *
     *  Inherits from BaseRand to provide a uniform distribution of real numbers
     *  in a specified range using the Mersenne Twister engine.
     */
    class RealRand : public BaseRand {
    public:
        /**
         *  @brief Default constructor, generates values in range [0.0, 1.0].
         */
        RealRand() noexcept : BaseRand(true) {
            setup(0.0, 1.0);
        }

        /**
         *  @brief Constructor with custom range.
         *  @param min Minimum value.
         *  @param max Maximum value.
         *  @param seed_flag Whether to seed the generator (default true).
         */
        RealRand(float min, float max, bool seed_flag = true) noexcept
            : BaseRand(seed_flag) {
            setup(min, max);
        }

        /**
         *  @brief Set up the distribution range.
         *  @param min Minimum value.
         *  @param max Maximum value.
         */
        void setup(float min, float max) noexcept {
            m_min = min;
            m_max = max;
            m_distribution = std::uniform_real_distribution<float>(min, max);
        }

        /**
         *  @brief Get the minimum bound of the distribution.
         */
        [[nodiscard]] float min() const noexcept { return m_min; }

        /**
         *  @brief Get the maximum bound of the distribution.
         */
        [[nodiscard]] float max() const noexcept { return m_max; }

        /**
         *  @brief Generate the next random value in the configured range.
         */
        [[nodiscard]] float next() noexcept override {
            return m_distribution(m_generator);
        }

    protected:
        std::uniform_real_distribution<float> m_distribution;
        float m_min = 0.0f;
        float m_max = 1.0f;
    };


    /**
     *  @class WhiteNoiseRand
     *  @brief Generates white noise samples uniformly distributed in the range [-1, 1].
     *
     *  Inherits from BaseRand and uses a Mersenne Twister engine with a uniform
     *  real distribution to produce white noise values. Commonly used in audio
     *  synthesis, stochastic simulations, or procedural generation.
     */
    class WhiteNoiseRand : public BaseRand {
    public:
        /**
         *  @brief Constructs the white noise generator and seeds it.
         */
        WhiteNoiseRand() noexcept : BaseRand(true) {
            setup();
        }

        /**
         *  @brief Initializes the uniform distribution range.
         */
        void setup() noexcept {
            m_distribution = std::uniform_real_distribution<float>(-1.0f, 1.0f);
        }

        /**
         *  @brief Returns the next white noise sample in the range [-1.0, 1.0].
         *  @return A uniformly distributed float between -1.0 and 1.0.
         */
        [[nodiscard]] float next() noexcept override {
            return static_cast<float>(m_distribution(m_generator));
        }

    protected:
        std::uniform_real_distribution<float> m_distribution;    ///< Uniform distribution [-1, 1]
    };


    /**
     *  @class GaussianWhiteNoiseRand
     *  @brief Generates normally distributed (Gaussian) random values.
     *
     *  This class inherits from BaseRand and provides values sampled from a
     *  Gaussian distribution (a.k.a. normal distribution), characterized by a
     *  configurable mean and standard deviation.
     *
     *  Useful in probabilistic modeling, noise generation, signal processing,
     *  and other simulations requiring normally distributed randomness.
     */
    class GaussianWhiteNoiseRand : public BaseRand {
    public:
        /**
         *  @brief Constructs a standard Gaussian generator (mean = 0.0, stddev = 1.0)
         *         and seeds the engine.
         */
        GaussianWhiteNoiseRand() noexcept : BaseRand(true) {
            setup(0.0, 1.0);
        }

        /**
         *  @brief Constructs with a custom mean and standard deviation, with seeding.
         *  @param mean Mean of the distribution.
         *  @param stddev Standard deviation of the distribution.
         */
        GaussianWhiteNoiseRand(float mean, float stddev) noexcept : BaseRand(true) {
            setup(mean, stddev);
        }

        /**
         *  @brief Sets up the normal distribution parameters and (re)initializes the distribution.
         *  @param mean Mean of the distribution.
         *  @param stddev Standard deviation.
         */
        void setup(float mean, float stddev) noexcept {
            m_mean = mean;
            m_stddev = stddev;
            m_normal_dist = std::normal_distribution<float>(m_mean, m_stddev);
        }

        /**
         *  @brief Generates the next normally distributed random number.
         *  @return A float sampled from N(mean, stddev²).
         */
        [[nodiscard]] float next() noexcept override {
            return m_normal_dist(m_generator);
        }

        [[nodiscard]] float mean() const noexcept { return m_mean; }
        [[nodiscard]] float stddev() const noexcept { return m_stddev; }

    protected:
        std::normal_distribution<float> m_normal_dist;   ///< Normal (Gaussian) distribution
        float m_mean = 0.0;                              ///< Mean of the distribution
        float m_stddev = 1.0;                            ///< Standard deviation of the distribution
    };


} // End of namespace Grain

#endif // GrainRandom_hpp
