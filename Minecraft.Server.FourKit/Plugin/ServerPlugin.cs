namespace Minecraft.Server.FourKit.Plugin;

/// <summary>
/// Base class that every plugin must extend.
/// <code>
/// public string name    => "MyPlugin";
/// public string version => "1.0.0";
/// public string author  => "Me";
///
/// public void onEnable()  { /* startup logic  */ }
/// public void onDisable() { /* shutdown logic */ }
/// </code>
/// </summary>
public abstract class ServerPlugin
{
    /// <summary>
    /// The name of this plugin. <b>Must be declared in your plugin class.</b>
    /// </summary>
    public virtual string name { get; } = string.Empty;

    /// <summary>
    /// The version of this plugin.
    /// </summary>
    public virtual string version { get; } = "1.0.0";

    /// <summary>
    /// The author of this plugin.
    /// </summary>
    public virtual string author { get; } = "Unknown";

    /// <summary>
    /// Called when this plugin is enabled
    /// </summary>
    public virtual void onEnable() { }

    /// <summary>
    /// Called when this plugin is disabled
    /// </summary>
    public virtual void onDisable() { }
}
