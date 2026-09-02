import { resolve } from 'node:path'
import { defineConfig } from 'vitepress'

type DocumentationVersion = {
  label: string
  url: string
}

function documentationVersions(): DocumentationVersion[] {
  const encoded = process.env.TABLE_DOCS_VERSIONS

  if (encoded === undefined) {
    return []
  }

  const parsed: unknown = JSON.parse(encoded)
  if (!Array.isArray(parsed)) {
    throw new Error('TABLE_DOCS_VERSIONS must be a JSON array')
  }

  return parsed.map((entry: unknown) => {
    if (
      typeof entry !== 'object' ||
      entry === null ||
      !('label' in entry) ||
      !('url' in entry) ||
      typeof entry.label !== 'string' ||
      typeof entry.url !== 'string'
    ) {
      throw new Error('TABLE_DOCS_VERSIONS contains an invalid entry')
    }

    return { label: entry.label, url: entry.url }
  })
}

const currentVersion = process.env.TABLE_DOCS_VERSION ?? 'Development'
const sourceRef = process.env.TABLE_DOCS_SOURCE_REF ?? 'main'
const outputDirectory = resolve(
  process.cwd(),
  process.env.TABLE_DOCS_OUT_DIR ?? 'build/docs/site'
)
const versions = documentationVersions()
const versionNavigation =
  versions.length === 0
    ? []
    : [
        {
          text: currentVersion,
          items: versions.map((version) => ({
            text: version.label,
            link: version.url,
            target: '_self',
            noIcon: true
          }))
        }
      ]
const sourceLink =
  sourceRef === 'main'
    ? {
        pattern: 'https://github.com/epicecu/table/edit/main/docs/:path',
        text: 'Edit this page on GitHub'
      }
    : {
        pattern: `https://github.com/epicecu/table/blob/${sourceRef}/docs/:path`,
        text: 'View source for this version'
      }

export default defineConfig({
  outDir: outputDirectory,
  title: 'Table.h',
  titleTemplate: ':title | EpicECU Table.h',
  description: 'Heap-free lookup curves and maps for embedded C, C++, and Rust',
  lang: 'en-AU',
  cleanUrls: true,
  lastUpdated: true,
  head: [['meta', { name: 'theme-color', content: '#ef5b2a' }]],
  themeConfig: {
    logo: '/epicecu-tables-logo.png',
    siteTitle: 'Table.h',
    search: { provider: 'local' },
    nav: [
      { text: 'Guide', link: '/introduction' },
      { text: 'Examples', link: '/examples' },
      { text: 'API', link: '/reference/c/' },
      ...versionNavigation,
      { text: 'GitHub', link: 'https://github.com/epicecu/table' }
    ],
    sidebar: [
      {
        text: 'Guide',
        items: [
          { text: 'Introduction', link: '/introduction' },
          { text: 'Installation', link: '/installation' },
          { text: 'Examples', link: '/examples' },
          { text: 'API design', link: '/api' },
          { text: 'Protobuf and Nanopb', link: '/protobuf' },
          { text: 'Migration', link: '/migration' }
        ]
      },
      {
        text: 'C API',
        collapsed: false,
        items: [
          { text: 'Overview', link: '/reference/c/' },
          { text: 'Types and status', link: '/reference/c/types' },
          { text: 'Initialisation', link: '/reference/c/initialisation' },
          { text: 'Lookup and access', link: '/reference/c/lookup' },
          { text: 'Mutation', link: '/reference/c/mutation' }
        ]
      },
      {
        text: 'C++ API',
        collapsed: false,
        items: [
          { text: 'Overview', link: '/reference/cpp/' },
          { text: 'Status', link: '/reference/cpp/status' },
          { text: 'Curve', link: '/reference/cpp/curve' },
          { text: 'Map', link: '/reference/cpp/map' }
        ]
      },
      {
        text: 'Rust API',
        collapsed: false,
        items: [
          { text: 'Overview', link: '/reference/rust/' },
          { text: 'Curve', link: '/reference/rust/curve' },
          { text: 'Map', link: '/reference/rust/map' },
          { text: 'Errors and scalars', link: '/reference/rust/types' },
          { text: 'Protobuf', link: '/reference/rust/protobuf' },
          { text: 'Low-level table-sys', link: '/reference/rust/table-sys' }
        ]
      },
      {
        text: 'Project',
        items: [
          { text: 'MISRA analysis', link: '/misra' },
          { text: 'Source style', link: '/style' },
          { text: 'Contributing', link: '/contributing' },
          { text: 'Security', link: '/security' }
        ]
      }
    ],
    editLink: sourceLink,
    socialLinks: [{ icon: 'github', link: 'https://github.com/epicecu/table' }],
    footer: {
      message: 'Released under the MIT Licence.',
      copyright: 'Copyright © 2026 EpicECU Pty Ltd'
    }
  }
})
