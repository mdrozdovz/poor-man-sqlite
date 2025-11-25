const std = @import("std");

pub fn build(b: *std.Build) void {
    // Standard build options for target and optimization mode
    const target = b.standardTargetOptions(.{});
    const mode = b.standardOptimizeOption(.{});

    const module = b.addModule("main", .{
        .target = target,
        .optimize = mode,
        .link_libc = true,
    });

    var sources = std.ArrayList([]const u8).initCapacity(b.allocator, 256) catch unreachable;
    {
        var dir = std.fs.cwd().openDir("src", .{ .iterate = true }) catch unreachable;

        var walker = dir.walk(b.allocator) catch unreachable;
        defer walker.deinit();

        const allowed_exts = [_][]const u8{
            ".c",
        };
        while (walker.next()  catch unreachable) |entry| {
            const ext = std.fs.path.extension(entry.basename);
            const include_file = for (allowed_exts) |e| {
                if (std.mem.eql(u8, ext, e))
                    break true;
            } else false;
            if (include_file) {
                // we have to clone the path as walker.next() or walker.deinit() will override/kill it
                sources.append(b.allocator, b.pathJoin(&.{"src", entry.path}))  catch unreachable;
            }
        }
    }

    module.addCSourceFiles(.{
        .files = sources.items,
        .flags = &[_][]const u8{"-std=c99"},
    });

    module.addIncludePath(b.path("include"));

    const exe = b.addExecutable(.{
        .name = "dbview",
        .root_module = module,
    });

    b.installArtifact(exe);

    const run_cmd = b.addRunArtifact(exe);
    run_cmd.step.dependOn(b.getInstallStep());

    if (b.args) |args| {
        run_cmd.addArgs(args);
    }

    const run_step = b.step("run", "Run the app");
    run_step.dependOn(&run_cmd.step);
}
