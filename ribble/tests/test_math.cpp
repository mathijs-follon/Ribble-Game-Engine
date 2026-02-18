#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

TEST_SUITE("Math Utilities") {
    TEST_CASE("Vector operations") {
        glm::vec3 a(1.0f, 2.0f, 3.0f);
        glm::vec3 b(4.0f, 5.0f, 6.0f);
        
        glm::vec3 sum = a + b;
        CHECK(sum.x == doctest::Approx(5.0f));
        CHECK(sum.y == doctest::Approx(7.0f));
        CHECK(sum.z == doctest::Approx(9.0f));
        
        glm::vec3 diff = b - a;
        CHECK(diff.x == doctest::Approx(3.0f));
        CHECK(diff.y == doctest::Approx(3.0f));
        CHECK(diff.z == doctest::Approx(3.0f));
    }
    
    TEST_CASE("Vector length") {
        glm::vec3 v(3.0f, 4.0f, 0.0f);
        float length = glm::length(v);
        CHECK(length == doctest::Approx(5.0f));
    }
    
    TEST_CASE("Vector normalization") {
        glm::vec3 v(3.0f, 4.0f, 0.0f);
        glm::vec3 normalized = glm::normalize(v);
        float length = glm::length(normalized);
        CHECK(length == doctest::Approx(1.0f));
    }
    
    TEST_CASE("Matrix multiplication") {
        glm::mat4 identity(1.0f);
        glm::mat4 scale = glm::scale(identity, glm::vec3(2.0f, 2.0f, 2.0f));
        
        glm::vec4 point(1.0f, 1.0f, 1.0f, 1.0f);
        glm::vec4 transformed = scale * point;
        
        CHECK(transformed.x == doctest::Approx(2.0f));
        CHECK(transformed.y == doctest::Approx(2.0f));
        CHECK(transformed.z == doctest::Approx(2.0f));
    }
    
    TEST_CASE("Matrix identity") {
        glm::mat4 identity(1.0f);
        glm::vec4 point(1.0f, 2.0f, 3.0f, 1.0f);
        glm::vec4 result = identity * point;
        
        CHECK(result.x == doctest::Approx(1.0f));
        CHECK(result.y == doctest::Approx(2.0f));
        CHECK(result.z == doctest::Approx(3.0f));
    }
}

