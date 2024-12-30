#include "drawing.hpp"

unsigned int TILE_SHADER, COLOR_SHADER;
Canvas* Canvas::instancePtr = nullptr;

Canvas::Canvas()
{
    initialize_window();
    initialize_shaders();
}

void Canvas::initialize_window()
{
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return;
    }

    int window_size[2];

    window_size[0] = 1600; //sim.x_size * tile_size + window_margin * 2 + (button_size + window_margin) * (((button_count - 1) / button_rows) + 1);
    window_size[1] = 900; //sim.y_size * tile_size + window_margin * 2;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create a windowed mode window and its OpenGL context
    window = glfwCreateWindow(
        window_size[0],
        window_size[1],
        "Sand",
        nullptr,
        nullptr
    );

    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = GL_TRUE; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return;
    }
}

void Canvas::initialize_shaders()
{
    TILE_SHADER = create_shader("tiles_fragment.glsl", "default_vertex.glsl");
    COLOR_SHADER = create_shader("color_fragment.glsl", "default_vertex.glsl");
}

unsigned int Canvas::compile_shader(unsigned int type, const char* source)
{
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    return shader;
}

unsigned int Canvas::create_shader(const char* fragment, const char* vertex)
{
    std::ifstream inputFile(vertex);
    std::string fileContent;
    std::string line;
    while (std::getline(inputFile, line)) {
        fileContent += line + "\n";
    }
    inputFile.close();

    unsigned int vertexShader = compile_shader(GL_VERTEX_SHADER, fileContent.c_str());

    inputFile.open(fragment);
    fileContent.clear();
    while (std::getline(inputFile, line)) {
        fileContent += line + "\n";
    }
    inputFile.close();
    unsigned int fragmentShader = compile_shader(GL_FRAGMENT_SHADER, fileContent.c_str());

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

void Canvas::draw_rect(float x, float y, float width, float height, float r, float g, float b, unsigned int VAO)
{
    glUseProgram(COLOR_SHADER);

    // Set the color
    glUniform3f(glGetUniformLocation(COLOR_SHADER, "color"), r, g, b);

    // Create a transformation matrix for the rectangle
    float vertices[] = {
        -1.0f + x / size.x * 2.0f, 1.0f - y / size.y * 2.0f, 0,         // Bottom left
        -1.0f + (x + width) / size.x * 2.0f, 1.0f - y / size.y * 2.0f, 0, // Bottom right
        -1.0f + (x + width) / size.x * 2.0f, 1.0f - (y + height) / size.y * 2.0f, 0, // Top right
        -1.0f + x / size.x * 2.0f, 1.0f - (y + height) / size.y * 2.0f, 0, // Top left
    };

    unsigned int indices[] = {
        0, 1, 3,
        1, 2, 3
    };

    unsigned int EBO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(VAO); // Bind the VAO
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0); // Unbind the VAO
}