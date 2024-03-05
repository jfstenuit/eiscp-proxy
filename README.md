# eISCP Proxy

## One-Line Summary
Control your Integra/Onkyo/Teac devices (AVR, BD player, etc.) across different subnets, with the help of the eISCP protocol.

## About
eISCP Proxy facilitates the discovery and control of Integra, Onkyo, Tascam and Teac devices across different network segments. Using the eISCP (Enhanced Integra Serial Control Protocol), this proxy ensures seamless interaction with your audio-video receivers, Blu-ray players, and other compatible devices, even when they reside on separate subnets.

The eISCP protocol is well-documented and can be referred to for an in-depth understanding of the communication standards it adheres to: [eISCP Protocol Documentation](https://tascam.com/downloads/tascam/790/pa-r100_200_protocol.pdf).

## Operation Principle
eISCP Proxy maintains an inventory of all eISCP-capable devices by broadcasting probe packets across specified network interfaces. It listens for incoming probe packets and responds with the list of discovered devices, effectively bridging the discovery process across subnet boundaries. This setup simplifies device management in complex network environments by centralizing discovery to a single point, the eISCP Proxy, allowing standard TCP connections for subsequent control commands.

It's important to note that eISCP Proxy specializes in proxying discovery packets. Once devices are discovered, the remaining communication utilizes standard TCP connections, which does not require protocol-specific adaptations.

## Features
- Auto-discovery of eISCP devices across subnets
- Responds to probe packets with known device lists
- Simplifies device control in multi-network environments

## Installation

The eISCP Proxy can be installed by compiling the source code, which requires GNU Autotools for configuration. Follow these steps to compile and install eISCP Proxy from the source:

1. **Clone the repository**

   First, clone the eISCP Proxy repository to your local machine using git:

   ```bash
   git clone https://github.com/jfstenuit/eiscp-proxy.git
   cd eiscp-proxy
   ```
   
2. **Configure the package**

   Before compiling the source code, configure the package for your system:

   ```bash
   ./configure
   ```
   
   This script will check your system for all required dependencies and set up the Makefile accordingly. If you wish to install eISCP Proxy to a custom location, you can specify the --prefix option during configuration:

   ```bash
   ./configure --prefix=/path/to/your/installation/directory
   ```
   
3. **Compile the source code**

   Compile the eISCP Proxy using the make command:

   ```bash
   make
   ```

4. **Install the software**

   After successful compilation, install eISCP Proxy to your system (as root, if necessary):

   ```bash
   sudo make install
   ```

   This step will copy the binary and any required resources to the appropriate locations on your system as defined during the configuration step.

5. **Run eISCP Proxy**

   After installation, you can start eISCP Proxy with:

   ```bash
   eiscp-proxy -i <interfaces>
   ```
   
   Replace <interfaces> with a comma-separated list of network interfaces you want the proxy to monitor.

   For more information on command-line options, use the -h option to display help:

   ```bash
   eiscp-proxy -h
   ```

### Uninstall

If you wish to uninstall eISCP Proxy, you can do so by executing the following command from the source directory:

   ```bash
   sudo make uninstall
   ```
This command will remove all files installed by make install.

## Usage
Launch the eISCP Proxy with the following command-line options to tailor its operation to your network setup:

```
Usage: eiscp-proxy [OPTIONS]

Options:
-i <interfaces> Comma-separated list of interfaces (mandatory)
-d Enable debug mode
-t <timeout> Set timeout interval in seconds (other than 5 seconds)
-h Display this help and exit
```

For instance :

```
/usr/local/bin/eiscp-proxy -i eth0,eth1.10,eth1.20
```

## Contributing

I warmly welcome contributions from the community, be it in the form of bug reports, feature requests, documentation improvements, or code contributions. Here's how you can contribute to eISCP Proxy:

### Reporting Bugs

- **Use the GitHub Issues page** - Before creating a new issue, please search existing issues to avoid duplicates.
- **Be clear and descriptive** - Include as much information as possible; steps to reproduce the bug, the expected outcome, and the actual outcome.
- **Include logs or error messages** if applicable.

### Feature Requests

- **Use the GitHub Issues page** - Clearly describe the feature and why it would be beneficial to the project.
- **Engage in discussions** - Be open to feedback from maintainers and other community members.

### Code Contributions

1. **Fork the repository** - Create your own fork of the eISCP Proxy project.
2. **Create a new branch** for your contribution.
3. **Follow the coding style** - Adhere to the existing coding conventions used in the project.
4. **Write meaningful commit messages** - Your messages should clearly describe the changes made.
5. **Test your changes** - Ensure that your code does not introduce any regressions or bugs.
6. **Submit a Pull Request (PR)** - Include a clear description of the changes and reference any related issue(s).
7. **Participate in the review process** - Be open to making changes based on feedback.

### Documentation

Improvements to the documentation are greatly appreciated. Whether it's a typo fix, additional content, or clearer examples, please feel free to submit PRs for documentation just like code contributions.

### Code of Conduct

- **Be respectful** to each other. Discrimination, harassment, or aggressive behaviors won't be tolerated.
- **Work transparently** - Discuss major changes with the community before implementing.
- **Acknowledge contributions** - Give credit where it's due.

### License

By contributing, you agree that your contributions will be licensed under the GPL-3.0 License.

### Getting Help

If you have any questions or need help with your contribution, please feel free to reach out to the maintainers or ask the community on the Issues page.


## License
This project is licensed under the GNU General Public License v3.0 - see the [LICENSE](LICENSE) file for details.
