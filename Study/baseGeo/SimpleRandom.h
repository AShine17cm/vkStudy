/* 生成随机数 */
#pragma once
#include <random>
#include <glm.hpp>
struct Random
{
	std::default_random_engine* engine;
	std::uniform_real_distribution<float>* dist;
	glm::vec2 range;
	Random(int seed,glm::vec2 range) 
	{
		engine = new std::default_random_engine(seed);
		this->range = range;
		dist = new std::uniform_real_distribution<float>(range.x, range.y);
	}
	float generate() 
	{
		return (*dist)(*engine);
	}
	~Random()
	{
		delete(dist);
		delete(engine);
	}
};