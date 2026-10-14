#include "Util.h"

#include <fstream>

std::string dodo::util::read_file(const std::string& file_path)
{
	std::ifstream in(file_path);
	if (!in.good())
		return "";

	in.seekg(0, std::ios::end);
	const size_t len = in.tellg();
	in.seekg(0);
	std::string str(len + 1, '\0');
	in.read(&str[0], len);
	in.close();
	return str;
}

std::string dodo::util::format_gl_debug_output(GLenum source, GLenum type, GLuint id, GLenum severity, const char* msg)
{
	std::stringstream stringStream;
	std::string sourceString;
	std::string typeString;
	std::string severityString;

	switch (source)
	{
	case GL_DEBUG_SOURCE_API:
		{
			sourceString = "API";
			break;
		}
	case GL_DEBUG_SOURCE_APPLICATION:
		{
			sourceString = "Application";
			break;
		}
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
		{
			sourceString = "Window System";
			break;
		}
	case GL_DEBUG_SOURCE_SHADER_COMPILER:
		{
			sourceString = "Shader Compiler";
			break;
		}
	case GL_DEBUG_SOURCE_THIRD_PARTY:
		{
			sourceString = "Third Party";
			break;
		}
	case GL_DEBUG_SOURCE_OTHER:
		{
			sourceString = "Other";
			break;
		}
	default:
		{
			sourceString = "Unknown";
			break;
		}
	}

	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:
		{
			typeString = "Error";
			break;
		}
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		{
			typeString = "Deprecated Behavior";
			break;
		}
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		{
			typeString = "Undefined Behavior";
			break;
		}
	// case GL_DEBUG_TYPE_PORTABILITY_ARB:
	// 	{
	// 		typeString = "Portability";
	// 		break;
	// 	}
	case GL_DEBUG_TYPE_PERFORMANCE:
		{
			typeString = "Performance";
			break;
		}
	case GL_DEBUG_TYPE_OTHER:
		{
			typeString = "Other";
			break;
		}
	default:
		{
			typeString = "Unknown";
			break;
		}
	}

	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:
		{
			severityString = "High";
			break;
		}
	case GL_DEBUG_SEVERITY_MEDIUM:
		{
			severityString = "Medium";
			break;
		}
	case GL_DEBUG_SEVERITY_LOW:
		{
			severityString = "Low";
			break;
		}
	default:
		{
			severityString = "Unknown";
			break;
		}
	}

	stringStream << "OpenGL Error: " << msg;
	stringStream << " [Source = " << sourceString;
	stringStream << ", Type = " << typeString;
	stringStream << ", Severity = " << severityString;
	stringStream << ", ID = " << id << "]";

	return stringStream.str();
}

glm::vec3 dodo::util::spherical_to_cartesian(float azimuth, float zenith, float radius)
{
	return glm::vec3{
		radius * sinf(zenith) * cosf(azimuth),
		radius * sinf(zenith) * sinf(azimuth),
		radius * cosf(zenith)
	};
}
